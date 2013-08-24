#include "nLogger.hpp"
#include <vector>
#include <cstdio>

namespace logger{
  struct LogMsg{
    int           type;
    std::string   msg;
  };
  typedef std::vector<LogMsg>   LogBuffer;
  typedef std::vector<LogOut>   LogOutVector;
  
  LogOutVector  g_outputs;
  LogBuffer     g_buffer;
  
  //------------------------------------------------------------------//
  void add_output( LogOut out ){
    g_outputs.push_back( out );
    for( const auto &entry: g_buffer ){
      out( entry.type, entry.msg );
    }
  }
  //------------------------------------------------------------------//
  void log( int type, const std::string &msg ){
    for( auto &out: g_outputs ){
      out( type, msg );
    }
    g_buffer.push_back( LogMsg{ type, msg } );
  }

  //------------------------------------------------------------------//
  void stdout( int type, const std::string &msg ){
    const char *tag;
    switch(type){
      case kDebug :     tag = "debug";    break;
      case kInfo :      tag = "info";     break;
      case kWarning :   tag = "warning";  break;
      case kError :     tag = "error";    break;
      default:          tag = "game";     break;
    };
    printf("%10s | %s", tag, msg.c_str());
  }
}

