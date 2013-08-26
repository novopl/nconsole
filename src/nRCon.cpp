#include "nRCon.hpp"
#include <nLogger.hpp>
#include <nConsole.hpp>

#include <cassert>
#include <cstring>      // memset
#include <cstdio>       // for InetAddr -> char*
#include <stdint.h>     
#include <unistd.h>
#include <sys/types.h>  // Required by some BSD socket.h implementations.
#include <sys/time.h>   // timeval
#include <sys/socket.h> // sockets funcs.
#include <arpa/inet.h>  // Byte order translations
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>      // select
#include <algorithm>

//{{{ novo::net
namespace novo{  namespace net{
  //------------------------------------------------------------------//
  template<typename Iterator>
  ssize_t write( Iterator &it, const void *data, size_t size ){
    const char *p = (char*)data;
    for( int i = 0; i < size; ++i ){
      *(it++) = *(p++);
    }
    return size;
  }
  //------------------------------------------------------------------//
  template<typename T, typename Iterator>
  ssize_t write( Iterator &it, const T &val ){
    const char *p = (char*)&val;
    for( int i = 0; i < sizeof(T); ++i ){
      *(it++) = *(p++);
    }
    return sizeof(T);
  }
  //------------------------------------------------------------------//
  template<typename T, typename Iterator>
  T read( Iterator &it ){
    auto val = *(T*)it;
    it += sizeof(T);
    return val;
  }
}}
//}}}

namespace novo{
  const int kInvalidSocket = -1;
  //{{{############### utils ###############
  //------------------------------------------------------------------//
  void close_sock( int &sock ){
    if( sock != kInvalidSocket )
      ::close( sock );
    sock = kInvalidSocket;
  };
  //------------------------------------------------------------------//
  ssize_t read_all( int &socket, std::vector<char> &buff){
    const size_t  kChunkSize  = 128;
    ssize_t       read        = 0;
    ssize_t       total       = 0;
  
    while( true ){
      char chunk[kChunkSize] = {0};
      read = recv( socket, chunk, kChunkSize, 0 );
  
      if( read > 0 ){
        for( int i = 0; i < read; ++i )
          buff.push_back( chunk[i] );
        total += read;
      }
      else if( read == 0 ){
        close_sock( socket );
        break;
      }
      else
        break;
    }
    return total;
  }
  //------------------------------------------------------------------//
  std::vector<char> read_all( int &socket ){
    std::vector<char> buff;
    read_all( socket, buff );
    return buff;
  }
  //------------------------------------------------------------------//
  std::string stripped(
      const std::string &str, const std::string &chars = "\t\r\n"
  ){
    std::string::size_type beg, end;
    beg = str.find_first_not_of( chars );
    end = str.find_last_not_of( chars );
    if( beg == std::string::npos )
      return std::string();
    else
      return str.substr( beg, end - beg + 1 );
  }
  //}}}


  //------------------------------------------------------------------//
  RConClient::RConClient() //{{{
  : m_port(0), 
    m_sock( kInvalidSocket )
  {
  }
  //}}}---------------------------------------------------------------//
  RConClient::~RConClient(){ //{{{
  }
  //}}}---------------------------------------------------------------//
  bool RConClient::connected() const{ //{{{
    return m_sock != kInvalidSocket;
  }
  //}}}---------------------------------------------------------------//
  void RConClient::connect( const std::string &host, int port ){ //{{{
    m_host =  host;
    m_port =  port;

    std::string pstr = std::to_string( m_port );
    logc( format("Connecting to %s:%s") %m_host %pstr );

    addrinfo hints, *srvinfo;
    memset( &hints, 0, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int ret = getaddrinfo(host.c_str(), pstr.c_str(), &hints, &srvinfo);
    if( ret != 0 ){
      logerr( format("Can't find server: %1%") %strerror(errno));
      return;
    }

    addrinfo *p;
    for( p = srvinfo; p != nullptr; p = p->ai_next ){
      m_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol );
      if( m_sock == kInvalidSocket ){
        logerr( format("Failed to create socket: %1%") %strerror(errno) );
        continue;
      }

      if( ::connect( m_sock, p->ai_addr, p->ai_addrlen ) == -1 ){
        logerr( format("Failed to connect: %1%") %strerror(errno) );
        close_sock( m_sock );
        continue;
      }
      fcntl( m_sock, F_SETFL, O_NONBLOCK );
      break;
    }

    if( p ) 
      logc("Connected");
    }
  //}}}---------------------------------------------------------------//
  void RConClient::disconnect(){ //{{{
    close_sock( m_sock );
    logf("rcon connection closed");
  }
  //}}}---------------------------------------------------------------//
  void RConClient::exec( const std::string &cmd ){ //{{{
    if( m_sock != kInvalidSocket ){
      int sent = ::send( m_sock, cmd.c_str(), cmd.length()+1, 0 );
      logd( format("sent cmd [%1% bytes]") %sent );
    }
  }
  //}}}---------------------------------------------------------------//
  void RConClient::update(){ //{{{
    // Read logs from the socket
    if( m_sock != kInvalidSocket ){
      fd_set    set;
      timeval   timeout;

      memset( &timeout, 0, sizeof(timeout) );
      FD_ZERO( &set );

      FD_SET( m_sock, &set );
      select( m_sock+1, &set, NULL, NULL, &timeout );

      if( FD_ISSET( m_sock, &set ) ){
        auto read = read_all( m_sock, m_buff );
        if( read > 0 )
          process_buffer();
        else if( read == 0 )
          logf( "Connection reset by peer" );
        else
          logc(format("read failed(%1%): %2%") %errno %strerror(errno));
      }
    }
  }
  //}}}---------------------------------------------------------------//
  void RConClient::process_buffer(){ //{{{
    const ptrdiff_t kHdrSize  = sizeof(int32_t) + sizeof(int64_t);

    char *fp  = &m_buff.front();
    char *end = &m_buff.back() + 1;
  
    //using novo::io::format_buff;
    //logd( 
    //    boost::format("Processing buffer:\n%1%\n") %format_buff(m_buff)
    //);
  
    while( (end-fp) > kHdrSize ){
      auto type = net::read<int32_t>( fp );
      auto when = net::read<int64_t>( fp );
      auto len  = net::read<int16_t>( fp );
      std::string msg;
      if( (end-fp) >= len ){
        char *tmp = new char[len+1];
        tmp[len]  = '\0';
        memcpy( tmp, fp, len );
        msg = tmp;
        delete tmp;
        fp += len;
      }
  
      //on_log( LogMsg{ type, when, msg } );
      lograw( type, when, msg );
    }
    int processed = fp - &m_buff.front();
    m_buff.erase( m_buff.begin(), m_buff.begin()+processed );
  }
  //}}}---------------------------------------------------------------//



  //------------------------------------------------------------------//
  RConServer::RConServer() //{{{
  : m_port(0),
    m_sockListen( kInvalidSocket ),
    m_sockConn( kInvalidSocket ),
    m_console( nullptr )
  {
  }
  //}}}---------------------------------------------------------------//
  RConServer::~RConServer(){ //{{{
    close_sock( m_sockConn );
    close_sock( m_sockListen );
  }
  //}}}---------------------------------------------------------------//
  void RConServer::bind( Console *console, int port ){ //{{{
    m_console = console;
    m_port    = port;

    try{
      if( (m_sockListen = socket( PF_INET, SOCK_STREAM, 0 )) == -1 )
        throw "socket() failed";

      int opt = 1;
      setsockopt(
          m_sockListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)
      );

      sockaddr_in addr;
      memset(&addr, 0, sizeof(sockaddr_in));
      addr.sin_addr.s_addr  = htons( INADDR_ANY );
      addr.sin_port         = htons( m_port );

      int ret = ::bind( m_sockListen, (const sockaddr*)&addr,
                        sizeof(sockaddr) );
      if( ret == -1 )
        throw "bind() failed";

      fcntl( m_sockListen, F_SETFL, O_NONBLOCK );

      if( listen( m_sockListen, 5 ) == -1 )
        throw "listen() failed";

    }catch( const char *ex ){
      close_sock( m_sockListen );
      logerr(ex);
      return;
    }
  }
  //}}}---------------------------------------------------------------//
  bool RConServer::connected() const{ //{{{
    return m_sockConn != kInvalidSocket;
  }
  //}}}---------------------------------------------------------------//
  void RConServer::update(){ //{{{
    using std::placeholders::_1;
    fd_set  set;
    timeval timeout;
    FD_ZERO( &set );
    memset( &timeout, 0, sizeof(timeout) );

    if( m_sockConn != kInvalidSocket ){
      // We have a connection
      // Check for commands.
      FD_SET( m_sockConn, &set );
      select( m_sockConn+1, &set, NULL, NULL, &timeout );
      if( FD_ISSET( m_sockConn, &set ) ){
        std::vector<char> buff = read_all( m_sockConn );
        if( m_sockConn != kInvalidSocket ){
          std::string msg( &buff.front() );
          logd( "RCON] " + msg );
          m_console->process_input( stripped(msg) );
        }
        else{
          logd("rcon connection reset by peer");
          remove_log_output( "rcon" );
        }
      }
    }
    else{
      // No one has connected yet
      FD_SET( m_sockListen, &set );
      select( m_sockListen+1, &set, NULL, NULL, &timeout );

      if( FD_ISSET( m_sockListen, &set ) ){
        sockaddr  addrRemote;
        socklen_t addrSize = sizeof( addrRemote );
        m_sockConn  = accept( m_sockListen, &addrRemote, &addrSize );
        if( m_sockConn != kInvalidSocket ){
          fcntl( m_sockConn, F_SETFL, O_NONBLOCK );
          logd( "rcon connected" );
          add_log_output("rcon",std::bind(&RConServer::send, this, _1));
        }
        else{
          logerr("accept() failed");
          m_sockConn = 0;
        }
      }
    }
  }
  //}}}---------------------------------------------------------------//
  void RConServer::send( const LogMsg &msg ){ //{{{
    assert( m_sockConn > 0 );
    std::vector<char> buff;
    auto it = std::back_inserter(buff);
    net::write<int32_t>( it, msg.type );
    net::write<int64_t>( it, msg.when );
    net::write<int16_t>( it, msg.msg.length() );
    net::write( it, msg.msg.c_str(), msg.msg.length() );
    int sent = ::send( m_sockConn, &buff.front(), buff.size(), 0 );
  }
  //}}}
}

