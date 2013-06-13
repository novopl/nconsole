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
int main(int argc, char **argv){
  novo::logger_init();
  novo::logger()->add_output( new novo::LogStdOut() );
  QApplication app(argc, argv);
  
  
  auto *console     = new novo::Console();
  auto *mainWindow  = new novo::QtConsole( console );

  //console->add( &test_cmd, "test", "Test command");
  console->add("test", "Test command", [&](const novo::String& args){
    novo::cprint("TEST COMMAND:\n");
    novo::CArgs cargs;
    console->tokenize( &cargs, args );
    for( const auto &arg: cargs ){
      novo::cprint( " - %s\n", arg.c_str() );
    }
  });
  
  mainWindow->show();

  int ret=app.exec();

  delete console;
  novo::logger_cleanup();

  return ret;
}
