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
#include <vector>
#include <functional>
#include <QWidget>
#include <QLabel>

class QLabel;
class QLineEdit;
class QTextEdit;
class QPushButton;

namespace novo{
  class LogMsg;
  class RConClient;
}

class QtConsole : public QWidget{
  QtConsole(const QtConsole &obj)             = delete;
  QtConsole& operator=(const QtConsole &obj)  = delete;
  Q_OBJECT
  
public:
  QtConsole(novo::Console *console);
  virtual ~QtConsole();
  
  void on_log( const novo::LogMsg &msg );

private slots:
  void on_input();
  void on_output_focus();
  void on_update();
  void on_connect();

private:
  QTextEdit     *m_output;
  QLineEdit     *m_input;
  QTimer        *m_timer;
  QLineEdit     *m_addr;
  QLineEdit     *m_port;
  QPushButton   *m_connect;
  
  int               m_sock;
  std::vector<char> m_buff;
  novo::Console     *m_console;
  novo::RConClient  *m_rcon;
};


#endif/* __NOVO_QTCONSOLE_HPP__ */
