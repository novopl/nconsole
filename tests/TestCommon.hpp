//==============================================================================
/**
    \file            TestCommon.hpp
    \author          Mateusz 'novo' Klos
    \date            10 January, 2010

  Stuff common to all tests.

 Copyright (c) 2007-2010 Mateusz 'novo' Klos
*/
//==============================================================================
#if !defined(__NSTD_TESTCOMMON_HPP__)
#define __NSTD_TESTCOMMON_HPP__

#include <cxxtest/TestSuite.h>
#include <cstdarg>
#include <cstdio>

#if !defined(_MSCVER)
#  define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#if defined(N_MUTE_TEST)
#  define N_TEST_SETUP()  log_stream()=0;
#else
#  define N_TEST_SETUP()
#endif

#define N_TEST_INFO()                                               \
    tlog( "\n"                                                      \
          "//=================================================\n"   \
          "// %s\n"                                                 \
          "//=================================================\n",  \
          __FUNCSIG__ );

inline FILE*& log_stream(){
  static FILE* logHandle=stdout;
  return logHandle;
}
inline void tlog(const char *fmt, ...){
  if( !log_stream() )
    return;
  
  va_list argList;
  va_start(argList, fmt);

  const int BUFF_SIZES[]={1024, 16*1024, 64*1024, 256*1024, 1024*1024, 16*1024*1024};
  int buffCount=sizeof(BUFF_SIZES)/sizeof(BUFF_SIZES[0]);

  char *ret[]={0, 0, 0, 0, 0, 0};

  int buffSize=0;
  int err=-1;
  int buffIndex=0;
  for(; err==-1 && buffIndex < buffCount; ++buffIndex){
    buffSize=BUFF_SIZES[buffIndex];

    if(buffIndex!=0)
      delete[] ret[buffIndex-1];
    
    ret[buffIndex]=new char[buffSize];

    err=vsnprintf(ret[buffIndex], buffSize-1, fmt, argList);
  }
  fputs(ret[buffIndex-1], log_stream());
  delete[] ret[buffIndex-1];
}

class LogFixture : public CxxTest::GlobalFixture{
public:
    bool setUpWorld(){
      const char *logOut="stdout";
#if defined(N_LOG_FILE)
      // log to file
      logOut=N_LOG_FILE;
      log_stream()=fopen(logOut, "w");
#else
      // log to stdout.
      tlog("\n");
#endif
      time_t curr=time(NULL);
      tlog("//==========================================================\n"
           "/**\n"
           " \\file         %s\n"
           " \\date:        %s"
           "*/\n"
           "//==========================================================\n",
           logOut,
           ctime(&curr)
           );
      return true;
    }
    bool tearDownWorld(){
      tlog("\n\n" );
      tlog("//==========================================================\n" );
      tlog("//                       -=[ Log end ]=-                    \n" );
      tlog("//==========================================================\n" );

#if defined (N_LOG_FILE)
      fclose(log_stream());
#endif
      log_stream()=stdout;
      
      return true;
    }
};
// Each test suite has it's own compilation unit, so
// the file is included only once, hence
// the definition can be here.
static LogFixture gs_logFixture;

#endif/* __NSTD_TESTCOMMON_HPP__ */
