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
#include <fd/delegate.hpp>
#include <QWidget>
#include <QLabel>

class QLabel;
class QLineEdit;
class QTextEdit;

namespace novo{
  //====================================================================
  /** \class QtLogOut
  \brief  Converts nlibs log events into Qt signals.
  */
  //====================================================================
  class QtLogOut : public LogOut{
      QtLogOut(const QtLogOut &obj)             {               }
      QtLogOut& operator=(const QtLogOut &obj)  { return *this; }
      typedef fd::delegate<void(const QString &msg)>  MsgCallback;
      
    public:
      QtLogOut();
      virtual ~QtLogOut();

      void set_callback(MsgCallback callback);
      virtual void log(const LogMessage &msg);
      
      
    private:
      MsgCallback     m_callback;
  };

  
  //====================================================================
  /** \class QtConsole
  \brief  Console Qt GUI.
  */
  //====================================================================
  class QtConsole : public QWidget{
      QtConsole(const QtConsole &obj)             {               }
      QtConsole& operator=(const QtConsole &obj)  { return *this; }
      Q_OBJECT
      
    public:
      QtConsole(Console *console);
      virtual ~QtConsole();
      
      void on_log(const QString &msg);

    private slots:
      void on_input();

    private:
      
      QtLogOut    *m_logOut;
      QTextEdit   *m_output;
      QLineEdit   *m_input;
      
      Console       *m_console;
  };


}

#endif/* __NOVO_QTCONSOLE_HPP__ */
