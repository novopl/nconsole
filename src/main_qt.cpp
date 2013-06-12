//==============================================================================
/**
\file            main_qt.cpp
\author          Mateusz 'novo' Klos
\date            July 26, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//==============================================================================
#include "nConsole.hpp"
#include "nQtConsole.hpp"
#include <nLogger.hpp>

#include <QApplication>


//--------------------------------------------------------------------//
int test_cmd(const novo::CArgs &args){
  using namespace novo;
  typedef CArgs::const_iterator Iter;
  cprint("TEST COMMAND:\n");
  for( Iter it = args.begin(); it != args.end(); ++it ){
    cprint( " - %s\n", it->c_str() );
  }
}


int main(int argc, char **argv){
  novo::logger_init();
  novo::logger()->add_output( new novo::LogStdOut() );
  QApplication app(argc, argv);
  
  
  auto *console     = new novo::Console();
  auto *mainWindow  = new novo::QtConsole( console );

  //console->add( &test_cmd, "test", "Test command");
  console->add("test", "Test command", [](const novo::CArgs& args){
    novo::cprint("TEST COMMAND:\n");
    for( const auto &arg: args ){
      novo::cprint( " - %s\n", arg.c_str() );
    }
  });
  
  mainWindow->show();

  int ret=app.exec();

  delete console;
  novo::logger_cleanup();

  return ret;
}
