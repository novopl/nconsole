#include "nLogger.hpp"
#include <vector>
#include <cstdio>

//------------------------------------------------------------------//
template<typename C, typename P> static 
typename C::iterator find( C &container, P pred ){
  return std::find_if( container.begin(), container.end(), pred );
}

#include <sys/time.h>
namespace novo{
  typedef int64_t Time_t;

  Time_t timezone(){
    static Time_t tzOffset = 0;
    if( !tzOffset ){
      tzset();
      time_t t = ::time(0);
      struct tm* data;
      data = ::localtime(&t);
      data->tm_isdst = 0;
      time_t a = ::mktime(data);
      data = ::gmtime(&t);
      data->tm_isdst = 0;
      time_t b = ::mktime(data);
      // The offset to the gmt in seconds.
      tzOffset = Time_t(a - b) * 1000000;
    }
    return tzOffset;
  }
  Time_t get_time(){
    timeval tv;
    gettimeofday(&tv, 0);
    return  static_cast<Time_t>(tv.tv_sec) * 1000000 + tv.tv_usec
            + timezone();
  }
}




namespace novo{

  struct LogOutDesc{
    std::string   name;
    LogOut        out;
  };
  typedef std::vector<LogOutDesc>   LogOutVector;
  
  LogOutVector  g_outs;
  LogBuffer     g_buffer;
  


  //------------------------------------------------------------------//
  void add_log_output( const std::string &name, LogOut out ){ //{{{
    auto it = find( g_outs, [&name]( LogOutDesc &d){
        return d.name == name;
    });

    if( it != g_outs.end() )
      log( kError, "Log output already exists: " + name );
    else{
      g_outs.push_back( LogOutDesc{ name, out } );
      for( const auto &entry: g_buffer )
        out( entry );
    }
  }
  //}}}---------------------------------------------------------------//
  void remove_log_output( const std::string &name ){ //{{{
    auto it = find( g_outs, [&name]( LogOutDesc &d){
        return d.name == name;
    });
    if( it != g_outs.end() )
      g_outs.erase( it );
  }
  //}}}---------------------------------------------------------------//
  void log( int type, const std::string &msg ){ //{{{
    lograw( type, get_time(), msg );
  }
  //}}}---------------------------------------------------------------//
  void lograw( int type, Time_t when, const std::string &msg ){ //{{{
    LogMsg  message{ type, when, msg };
    for( const auto &out: g_outs )
      out.out( message );
    g_buffer.push_back( message );
  }
  //}}}

  //------------------------------------------------------------------//
  void stdout( const LogMsg &msg ){ //{{{
    const char *tag;
    switch( msg.type ){
      case kDebug :     tag = "debug";    break;
      case kInfo :      tag = "info";     break;
      case kWarning :   tag = "warning";  break;
      case kError :     tag = "error";    break;
      default:          tag = "game";     break;
    };
    printf("%10s | %s", tag, msg.msg.c_str());
  }
  //}}}
}

