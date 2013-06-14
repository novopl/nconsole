//======================================================================
/**
\file            main_qt.cpp
\author          Mateusz 'novo' Klos
\date            July 26, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#include "nConsole.hpp"
#include "nQtConsole.hpp"
#include <nLogger.hpp>

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

#include <functional>
#include <boost/format.hpp>


//------------------------------------------------------------------//
QtConsole::QtConsole(novo::Console *console)
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

  m_timer = new QTimer();
  connect( m_timer, SIGNAL( timeout() ), this, SLOT( on_update() ) );
  m_timer->start(0);

  connect(m_input, SIGNAL(returnPressed()), this, SLOT(on_input()));
  connect(m_input, SLOT(setFocus()), this, SLOT(on_output_focus()));

  m_input->setFocus();
}
//------------------------------------------------------------------//
QtConsole::~QtConsole(){
  delete m_timer;
}
//------------------------------------------------------------------//
void QtConsole::on_update(){
  // Read logs from the socket
}
//------------------------------------------------------------------//
void QtConsole::on_input(){
  // Send input through the socket
  m_console->process_input( m_input->text().toStdString() );
  m_input->clear();
}
//------------------------------------------------------------------//
void QtConsole::on_log(int type, const std::string &msg){
  const char *fmt = 0;
  if( type & logger::kError )
    fmt = "<font color=\"#ff0000\">%s</font><br/>";
  else if( type & logger::kWarning )
    fmt = "<font color=\"#da8b05\">%s</font><br/>";
  else if( type & logger::kDebug )
    fmt = "<font color=\"#808080\">%s</font><br/>";
  else
    fmt = "<font color=\"#000000\">%s</font><br/>";

  m_output->append( QString(str( boost::format(fmt) %msg ).c_str()) );
}
//------------------------------------------------------------------//
void QtConsole::on_output_focus(){
  m_input->setFocus();
}



//--------------------------------------------------------------------//
int main(int argc, char **argv){
  using boost::format;
  using std::placeholders::_1;
  using std::placeholders::_2;

  logger::add_output( &logger::stdout );
  QApplication app(argc, argv);
  
  auto *console = new novo::Console();
  auto *window  = new QtConsole( console );

  logger::add_output( std::bind(&QtConsole::on_log, window, _1, _2) );

  using novo::String;
  console->on_change("var", [](const String &name, const String &value){
    novo::cprint( format("%s changed value to %s") %name %value );
  });
  console->add("test", "Test command", [&](const novo::String& args){
    novo::cprint("TEST COMMAND:\n");
    novo::CArgs cargs;
    console->tokenize( &cargs, args );
    for( const auto &arg: cargs ){
      novo::cprint( format(" - %s\n") %arg );
    }
  });
  
  window->show();

  int ret=app.exec();

  console->remove("test");
  delete console;
  return ret;
}
