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
int main(int argc, char **argv){
  using namespace novo; 
  using boost::format;
  
  novo::add_log_output( "stdout", &novo::stdout );
  //logger()->add_output( new LogStdOut() );
  //logger()->add_output( new LogFileOut("console.log"),  LogLevel::All );

  Console *console = new Console();

  console->add("test", "Test command", [&](const String &args){
    novo::logf("TEST COMMAND:\n");
    CArgs cargs;
    console->tokenize( &cargs, args );
    for( const auto &arg: cargs ){
      novo::logf(str( format(" - %s\n") %arg ));
    }
  });
  console->process_input("cmdlist");
  console->process_input("test arg1 arg2 \"multi arg3\"");
  console->process_input("help");
  console->process_input("help test");
  console->process_input("help wrong test");

  console->remove("test");
  delete console;

  return 0;
}
