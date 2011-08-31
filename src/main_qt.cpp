//==============================================================================
/**
\file            main_qt.cpp
\author          Mateusz 'novo' Klos
\date            July 26, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//==============================================================================
#include "Console.hpp"
#include "QtConsole.hpp"
#include "Logger.hpp"

#include <QApplication>

int main(int argc, char **argv){
  novo::logger_init();
  QApplication app(argc, argv);
  
  novo::Console *console    =new novo::Console();
  QWidget       *mainWindow =new novo::QtConsole(console);
  
  mainWindow->show();

  int ret=app.exec();

  delete console;
  novo::logger_cleanup();

  return ret;
}
