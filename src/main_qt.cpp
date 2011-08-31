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
int test_cmd(const novo::StringVector &args){
  using namespace novo;
  typedef StringVector::const_iterator Iter;
  cprint("TEST COMMAND:\n");
  for( Iter it = args.begin(); it != args.end(); ++it ){
    cprint( " - %s\n", it->c_str() );
  }
}


int main(int argc, char **argv){
  novo::logger_init();
  QApplication app(argc, argv);
  
  novo::Console *console    =new novo::Console();
  QWidget       *mainWindow =new novo::QtConsole(console);

  console->add( &test_cmd, "test", "Test command");
  
  mainWindow->show();

  int ret=app.exec();

  delete console;
  novo::logger_cleanup();

  return ret;
}
