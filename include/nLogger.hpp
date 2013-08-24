#if !defined(__NOVO_NLOGGER_HPP__)
#define __NOVO_NLOGGER_HPP__
#include <string>
#include <functional>

namespace logger{
  typedef std::function<void(int, const std::string&)>  LogOut;
  const int kDebug    = 0x0001;
  const int kInfo     = 0x0002;
  const int kWarning  = 0x0004;
  const int kError    = 0x0008;
  const int kGame     = 0x0010; // All higher bits are for game types

  const int kLogErrors    = kError;
  const int kLogWarnings  = kError | kWarning;
  const int kLogNormal    = 1 ^ kDebug;
  const int kLogDebug     = ~0;

  
  extern void add_output( LogOut out );
  extern void log( int type, const std::string &msg );

  // Helpers.
  inline void logd(const std::string &msg)  {return log(kDebug, msg);  }
  inline void logf(const std::string &msg)  {return log(kInfo, msg);   }
  inline void logw(const std::string &msg)  {return log(kWarning, msg);}
  inline void logerr(const std::string &msg){return log(kError, msg);  }

  extern void stdout( int type, const std::string &msg );
}
#endif/* __NOVO_NLOGGER_HPP__ */

