//======================================================================
/**
\file            QtConsole.cpp
\author          Mateusz 'novo' Klos
\date            July 26, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#include "nQtConsole.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

namespace novo{
  //====================================================================
  /** \class QtLogFormatter
  \brief  Log formatter for QtLogOut.
  */
  //====================================================================
  class LogHtmlFormatter : public LogFormatter{
    virtual Out format_message(const LogMessage &msg) const{
      const char *fmt = 0;
      if( msg.type & LogMsg::Error )
        fmt = "<font color=\"#ff0000\">%s</font><br/>";
      else if( msg.type & LogMsg::Warning )
        fmt = "<font color=\"#da8b05\">%s</font><br/>";
      else if( msg.type & LogMsg::Debug )
        fmt = "<font color=\"#808080\">%s</font><br/>";
      else
        fmt = "<font color=\"#000000\">%s</font><br/>";

      return Out( log_format(fmt, msg.msg) );
    }
  };
  //------------------------------------------------------------------//
  QtLogOut::QtLogOut()
  :LogOut( new LogHtmlFormatter() ){
    
  }
  //------------------------------------------------------------------//
  QtLogOut::~QtLogOut(){
    
  }
  //------------------------------------------------------------------//
  void QtLogOut::log(const LogMessage &msg){
    if( m_callback )
      m_callback( QString(formatter()->format_message(msg)) );
  }
  //------------------------------------------------------------------//
  void QtLogOut::set_callback(MsgCallback callback){
    m_callback = callback;
  }





  
  //------------------------------------------------------------------//
  QtConsole::QtConsole(Console *console)
  :m_console(console){
    m_output  =new QTextEdit();
    m_input   =new QLineEdit();

    m_output->setReadOnly(true);

    QVBoxLayout *layout =new QVBoxLayout();
    layout->addWidget(m_output);
    layout->addWidget(m_input);
    layout->setMargin(0);

    setLayout(layout);
    resize(600,400);
    //-- End of init.

    m_logOut  = new QtLogOut();
    //m_logOut->set_callback( fd::bind(&QtConsole::on_log, this)  );
    using std::placeholders::_1;
    m_logOut->set_callback( std::bind(&QtConsole::on_log, this, _1) );
    connect(m_input, SIGNAL(returnPressed()), this, SLOT(on_input()));
    connect(m_input, SLOT(setFocus()), this, SLOT(on_output_focus()));
    
    logger()->add_output( m_logOut, kConsoleMsg | LogLevel::Warning );

    m_input->setFocus();
  }
  //------------------------------------------------------------------//
  QtConsole::~QtConsole(){
  }
  //------------------------------------------------------------------//
  void QtConsole::on_input(){
    m_console->process_input( m_input->text().toStdString() );
    m_input->clear();
  }
  //------------------------------------------------------------------//
  void QtConsole::on_log(const QString &msg){
    m_output->append( msg );
  }
  //------------------------------------------------------------------//
  void QtConsole::on_output_focus(){
    m_input->setFocus();
  }
}
