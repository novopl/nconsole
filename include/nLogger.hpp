#if !defined(__NOVO_NLOGGER_HPP__)
#define __NOVO_NLOGGER_HPP__
#include <string>
#include <functional>
#include <boost/format.hpp>

namespace novo{
  using boost::format;
  typedef std::string String;
  typedef int64_t     Time_t;
  
  struct LogMsg{
    int           type;
    Time_t        when;
    std::string   msg;
  };

  typedef std::vector<LogMsg>                 LogBuffer;
  typedef std::function<void(const LogMsg&)>  LogOut;
  const int kDebug    = 0x0001;
  const int kInfo     = 0x0002;
  const int kWarning  = 0x0004;
  const int kError    = 0x0008;
  const int kGame     = 0x0010; // All higher bits are for game types

  const int kLogErrors    = kError;
  const int kLogWarnings  = kError | kWarning;
  const int kLogNormal    = 1 ^ kDebug;
  const int kLogDebug     = ~0;

  
  extern void add_log_output( const std::string &name, LogOut out );
  extern void remove_log_output( const std::string &name );
  extern void log( int type, const std::string &msg );
  extern void lograw( int type, Time_t when, const std::string &msg );

  // Helpers
  typedef boost::basic_format<char> Fmt;
  inline void logd(const String &msg)   { return log( kDebug,   msg+"\n");     }
  inline void logf(const String &msg)   { return log( kInfo,    msg+"\n");     }
  inline void logw(const String &msg)   { return log( kWarning, msg+"\n");     }
  inline void logerr(const String &msg) { return log( kError,   msg+"\n");     }
  inline void logd(const Fmt &msg)      { return log( kDebug,   str(msg)+"\n");}
  inline void logf(const Fmt &msg)      { return log( kInfo,    str(msg)+"\n");}
  inline void logw(const Fmt &msg)      { return log( kWarning, str(msg)+"\n");}
  inline void logerr(const Fmt &msg)    { return log( kError,   str(msg)+"\n");}


  extern void stdout( const LogMsg &msg );
}
#endif/* __NOVO_NLOGGER_HPP__ */

