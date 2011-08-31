//==============================================================================
/**
\file            main.cpp
\author          Mateusz 'novo' Klos
\date            pon, 12 lip 2010, 18:30:57 CEST, 2010

  

Copyright (c) 2010 Mateusz 'novo' Klos
*/
//==============================================================================
#include "nConsole.hpp"
#include "nConsoleTmp.hpp"
#include <nLogger.hpp>
#include "SDLFramework.hpp"
#include "FontRenderers.hpp"

#include <GL/gl.h>





//======================================================================
/** \class App
\brief  SDL framework application.
*/
//======================================================================
class App : public ngl::AppInterface{
  App(const App &obj)             {               }
  App& operator=(const App &obj)  { return *this; }

  public:
    App(){}
    virtual ~App(){}

    virtual int   init(int argc, char **argv);
    virtual int   cleanup();
    virtual int   tick();
    
    virtual int   on_event(::SDL_Event*);
    virtual int   on_resize(uint32_t w, uint32_t h);

  private:
    novo::Console         *m_console;
    novo::ConsoleUi       *m_consoleUi;
};
//--------------------------------------------------------------------//
int App::init(int argc, char **argv){
  using namespace novo;
  const int ConsoleLog  = LogLevel::App | LogLevel::Warning;
  logger_init();
  logger()->add_output(new LogStdOut( new LogPlainFormatter() ), ConsoleLog);
  
  ngl::init_extensions();
  ngl::freetype::init();

  m_console   =0;
  m_consoleUi =init_console_ui(ngl::Size2(800, 600), 200,
                               "Inconsolata.otf", 11,
                               m_console);
  
  m_console->process_input("lamka");
  m_console->process_input("echo lamka");


  SDL_EnableUNICODE(1);
  int uni = SDL_EnableUNICODE(-1);
  app_log("Unicode support %s\n", uni ? "enabled" : "disabled");
  
  return ngl::EOk;
}
//--------------------------------------------------------------------//
int App::cleanup(){
  ngl::freetype::cleanup();

  novo::logger()->remove_output(m_consoleUi);
  delete m_console;
  delete m_consoleUi;
  
  novo::logger_cleanup();
  
  return ngl::EOk;
}
//--------------------------------------------------------------------//
int App::tick(){
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  m_consoleUi->render();

  return ngl::EOk;
}
//--------------------------------------------------------------------//
int App::on_event(::SDL_Event *e){
  if( (e->type == SDL_KEYDOWN) || (e->type == SDL_KEYUP) )
    m_consoleUi->on_key( e->key );
  
  if( e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE ){
    novo::app_log("Quitting\n");
    return 1;
  }
  
  return ngl::EOk;
}
//--------------------------------------------------------------------//
int App::on_resize(uint32_t w, uint32_t h){
  return ngl::EOk;
}


  
int main(int argc, char **argv){
  return ngl::app::execute<App>(argc, argv);
}
