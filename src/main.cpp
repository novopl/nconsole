/*
 * =====================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  31.08.2011 14:32:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mateusz 'novo' Klos <novopl@gmail.com>
 *        Webpage:  novo.one.pl
 *        Company:  
 *
 * =====================================================================
 */
#include "nConsole.hpp"
#include <nLogger.hpp>

//--------------------------------------------------------------------//
int test_cmd(const novo::StringVector &args){
  using namespace novo;
  typedef StringVector::const_iterator Iter;
  log("TEST COMMAND:\n");
  for( Iter it = args.begin(); it != args.end(); ++it ){
    log( " - %s\n", it->c_str() );
  }
}

//--------------------------------------------------------------------//
int main(int argc, char **argv){
  using namespace novo; 
  logger_init();
  
  logger()->add_output( new LogStdOut() );
  logger()->add_output( new LogFileOut("console.log"),  LogLevel::All );

  Console *console = new Console();

  console->add( &test_cmd, "test", "Test command");
  console->process_input("cmdlist");
  console->process_input("test arg1 arg2 \"multi arg3\"");
  console->process_input("help");
  console->process_input("help test");
  console->process_input("help wrong test");

  logger_cleanup();
  return 0;
}
