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
  extern void logmsg( int type, const std::string &msg );
  extern void lograw( int type, Time_t when, const std::string &msg );

  // Helpers
  typedef boost::basic_format<char> Fmt;
  namespace suffix{
    inline boost::basic_format<char> operator "" _fmt (const char *str, size_t){
      return format(str);
    }
  }

  inline void logd(const String &msg)   { return logmsg( kDebug,   msg+"\n");  }
  inline void logf(const String &msg)   { return logmsg( kInfo,    msg+"\n");  }
  inline void log(const String &msg)    { return logmsg( kInfo,    msg+"\n");  }
  inline void logw(const String &msg)   { return logmsg( kWarning, msg+"\n");  }
  inline void logerr(const String &msg) { return logmsg( kError,   msg+"\n");  }
  inline void logd(const Fmt &msg)   { return logmsg( kDebug,   str(msg)+"\n");}
  inline void logf(const Fmt &msg)   { return logmsg( kInfo,    str(msg)+"\n");}
  inline void log(const Fmt &msg)    { return logmsg( kInfo,    str(msg)+"\n");}
  inline void logw(const Fmt &msg)   { return logmsg( kWarning, str(msg)+"\n");}
  inline void logerr(const Fmt &msg) { return logmsg( kError,   str(msg)+"\n");}


  extern void stdout( const LogMsg &msg );
}
#endif/* __NOVO_NLOGGER_HPP__ */

