//======================================================================
/**
\file            main_qt.cpp
\author          Mateusz 'novo' Klos
\date            July 26, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#include "rcon_qt.hpp"
#include "nConsole.hpp"
#include "nRCon.hpp"
#include <nLogger.hpp>

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTimer>

#include <functional>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

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
#include <cerrno>
#include <cstring>


//{{{ ###############   BinaryIO    ###############
#include <functional>
#include <stdint.h>

namespace novo{ namespace io{
  typedef std::function<ssize_t(const void *, size_t)>  OutFn;
  typedef std::function<ssize_t(void *, size_t)>        InFn;

  //====================================================================
  class Writer{ //{{{
  public:
    Writer( OutFn fn )
      :out( fn ){
    }

    ssize_t int8 (const int8_t  val)  { return out(&val, sizeof val); }
    ssize_t int16(const int16_t val)  { return out(&val, sizeof val); }
    ssize_t int32(const int32_t val)  { return out(&val, sizeof val); }
    ssize_t int64(const int64_t val)  { return out(&val, sizeof val); }


    ssize_t string( const std::string &str, bool terminate=true ){
      return out( str.c_str(), str.length()+terminate );
    }
    ssize_t string8( const std::string &str ){
      ssize_t written = int8( str.length() );
      return written + string( str, false );
    }
    ssize_t string16( const std::string &str ){
      ssize_t written = int16( str.length() );
      return written + string( str, false );
    }
    ssize_t string32( const std::string &str ){
      ssize_t written = int32( str.length() );
      return written + string( str, false );
    }

    ssize_t write( const void *data, size_t size ){
      return out( data, size );
    }
    template<typename T>
    ssize_t write( const T &data ){
      return out( &data, sizeof(T) );
    }

  private:
    OutFn   out;
  };
  //}}}


  //------------------------------------------------------------------//
  template<typename T, typename Iterator>
  T read( Iterator &it ){
    auto val = *(T*)it;
    it += sizeof(T);
    return val;
  }
  //------------------------------------------------------------------//
  std::string format_buff( const std::vector<char> &buff ){
    std::string out;
    ptrdiff_t fpval;
    auto last     = buff.size() % 16;
    auto fullRows = buff.size() / 16;
    for( int i = 0; i < fullRows; ++i ){
      fpval = buff.front()+ fullRows*16 ;
      out  += str(boost::format("0x%08x   ") %fpval );
      for( int j = 0; j < 16; ++ j ){
        unsigned int ch = (unsigned char)buff[i*16+j];
        out += str(boost::format("%02x ") %ch);
      }
      out += "  ";

      for( int j = 0; j < 16; ++ j ){
        char ch = buff[i*16+j];
        out += ch < 32 ? '#' : ch;
      }

      out += '\n';
    }

    fpval = buff.front()+ fullRows*16 ;
    out  += str(boost::format("0x%08x   ") %fpval );
    for( int j = 0; j < last; ++ j ){
      unsigned int ch = (unsigned char)buff[fullRows*16+j];
      out += str(boost::format("%02x ") %ch);
    }
    for( int j = 0; j < 16-last; ++ j ){
      out += "   ";
    }
    out += "  ";
    for( int j = 0; j < last; ++ j ){
      char ch = buff[fullRows*16+j];
      out += ch < 32 ? '#' : ch;
    }
    out += '\n';
    return out;
  };
}}
//}}}
using namespace novo;

//{{{############### Network utils ###############
int kInvalidSocket = -1;
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
      logf( "Connection reset by peer" );
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
//}}}

//------------------------------------------------------------------//
QtConsole::QtConsole(Console *console) //{{{
:   m_sock( kInvalidSocket ),
    m_console( console )
{
  m_rcon    = new RConClient();

  m_output  = new QTextEdit();
  m_input   = new QLineEdit();
  m_output->setFont( QFont("DejaVu Sans Mono", 9) );

  m_output->setReadOnly(true);

  auto *addrLay     = new QHBoxLayout();
  m_addr    = new QLineEdit();
  m_port    = new QLineEdit();
  m_connect = new QPushButton("connect");
  addrLay->addWidget( new QLabel("Address") );
  addrLay->addWidget( m_addr, 3);
  addrLay->addWidget( new QLabel("Port") );
  addrLay->addWidget( m_port, 1);
  addrLay->addWidget( m_connect);
  m_addr->setText("127.0.0.1");
  m_port->setText("37000");

  QVBoxLayout *layout =new QVBoxLayout();
  layout->addLayout( addrLay );
  layout->addWidget(m_output);
  layout->addWidget(m_input);
  layout->setMargin(0);

  setLayout(layout);
  resize(600,400);
  //-- End of init.

  m_timer = new QTimer();
  connect( m_timer, SIGNAL( timeout() ), this, SLOT( on_update() ) );
  m_timer->start(0);

  connect(m_input, SIGNAL(returnPressed()), this, SLOT(on_input()));
  connect(m_input, SLOT(setFocus()), this, SLOT(on_output_focus()));
  connect(m_connect, SIGNAL(clicked()), this, SLOT(on_connect()));

  m_input->setFocus();
}
//}}}---------------------------------------------------------------//
QtConsole::~QtConsole(){ //{{{
  delete m_rcon;
  delete m_timer;
}
//}}}---------------------------------------------------------------//
void QtConsole::on_connect(){ //{{{
  using boost::lexical_cast;
  std::string addr  = m_addr->text().toStdString();
  std::string port  = m_port->text().toStdString();
  logc( format("Connecting to %s:%s") %addr %port );

  if( m_rcon->connected() )
    m_rcon->disconnect();
  m_rcon->connect( addr, lexical_cast<int>(port) );
}
//}}}-----------------------------------------------------------------//
void QtConsole::on_update(){ //{{{
  if( m_rcon->connected() ){
    m_rcon->update();
    if( !m_rcon->connected() )
      logw("rcon connection reset by peer");
  }
}
//}}}---------------------------------------------------------------//
void QtConsole::on_input(){ //{{{
  // Send input through the socket
  auto input = m_input->text().toStdString();
  m_input->clear();
  m_rcon->exec( input );
}
//}}}---------------------------------------------------------------//
void QtConsole::on_log( const LogMsg &msg){ //{{{
  const char *fmt = 0;
  if( msg.type & kError )
    fmt = "<font color=\"#ff0000\">%s</font><br/>";
  else if( msg.type & kWarning )
    fmt = "<font color=\"#da8b05\">%s</font><br/>";
  else if( msg.type & kDebug )
    fmt = "<font color=\"#808080\">%s</font><br/>";
  else if( msg.type & kGame )
    fmt = "<font color=\"#008000\">%s</font><br/>";
  else
    fmt = "<font color=\"#000000\">%s</font><br/>";

  m_output->append( 
      QString(str( boost::format(fmt) %msg.msg ).c_str()) 
  );
}
//}}}---------------------------------------------------------------//
void QtConsole::on_output_focus(){ //{{{
  m_input->setFocus();
}
//}}}



//--------------------------------------------------------------------//
int main(int argc, char **argv){
  using namespace novo;
  using boost::format;
  using std::placeholders::_1;
  using std::placeholders::_2;

  add_log_output( "stdout", &novo::stdout );
  QApplication app(argc, argv);
  
  auto *console = new Console();
  auto *window  = new QtConsole( console );

  add_log_output( "qt", std::bind(&QtConsole::on_log, window, _1) );

  console->on_change("var", [](const String &name, const String &value){
    cprint( format("%s changed value to %s") %name %value );
  });
  console->add("test", "Test command", [&](const String& args){
    cprint("TEST COMMAND:\n");
    CArgs cargs;
    console->tokenize( &cargs, args );
    for( const auto &arg: cargs ){
      cprint( format(" - %s\n") %arg );
    }
  });
  
  window->show();

  int ret=app.exec();

  console->remove("test");
  delete console;
  return ret;
}
