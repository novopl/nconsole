//======================================================================
/**
\file            QtConsole.hpp
\author          Mateusz 'novo' Klos
\date            July 26, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#if !defined(__NOVO_QTCONSOLE_HPP__)
#define __NOVO_QTCONSOLE_HPP__

#include "nConsole.hpp"
#include "nLogger.hpp"
#include <functional>
#include <QWidget>
#include <QLabel>

class QLabel;
class QLineEdit;
class QTextEdit;

class QtConsole : public QWidget{
  QtConsole(const QtConsole &obj)             = delete;
  QtConsole& operator=(const QtConsole &obj)  = delete;
  Q_OBJECT
  
public:
  QtConsole(novo::Console *console);
  virtual ~QtConsole();
  
  void on_log(int type, const std::string &msg);

private slots:
  void on_input();
  void on_output_focus();
  void on_update();

private:
  QTextEdit     *m_output;
  QLineEdit     *m_input;
  QTimer        *m_timer;
  
  novo::Console *m_console;
};


#endif/* __NOVO_QTCONSOLE_HPP__ */
