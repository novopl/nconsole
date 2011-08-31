//======================================================================
/**
\file            ConsoleTmp.hpp
\author          Mateusz 'novo' Klos
\date            July 21, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#if !defined(__NOVO_CONSOLETMP_HPP__)
#define __NOVO_CONSOLETMP_HPP__

// #include "Console.hpp"
#include "Logger.hpp"
#include "Font.hpp"

#include <SDL/SDL.h>

namespace ngl{
  class AbstractRenderer;
}
namespace novo{
//======================================================================
/** \class LogStdFormatter
\brief  Log std formatter.
*/
//======================================================================
  class ConsoleFormatter: public LogFormatter{
  public:
    ConsoleFormatter(){}
    virtual ~ConsoleFormatter(){}

    virtual Out format_message(const LogMessage &msg) const{
      int color;
      switch(msg.type){
        case LogMsg::Error:     color = 1;    break;
        case LogMsg::Warning:   color = 15;   break;
        case LogMsg::App:       color = 0;    break;
        default:                color = 0;    break;
      }
      return Out( format("^%d%s", color, msg.msg) );
    }

  protected:
    ConsoleFormatter(const ConsoleFormatter &obj){
    }
    ConsoleFormatter& operator=(const ConsoleFormatter &obj){
      return *this;
    }

  private:
  };
//======================================================================
/** \class LineInputBuffer
\brief  
*/
//======================================================================
  class LineInputBuffer{
    public:
      typedef fd::delegate<void(const std::string&)>    Callback;
      struct CharCode;
      typedef uint32_t  Size_t;

      LineInputBuffer();
      virtual ~LineInputBuffer();
      
      void consume(const CharCode &code);
      void set_callback(Callback callback);
      
    private:
      typedef std::vector<std::string>    StringVector;
      String        m_line;
      Callback      m_onLine;
  };
  struct LineInputBuffer::CharCode{
    enum Type{
      Text    =0x01,
      Symbol  =0x02,
      Cmd     =0x04,
      Ascii   =Text | Symbol
    };
    union{
      uint16_t    code;
      struct{
        uint8_t type;
        char    ch;
      };
    };

    CharCode(const uint16_t _code)               :code(_code)         {}
    CharCode(const uint8_t _type, const char _ch):type(_type), ch(_ch){}
    operator uint16_t() const                    { return code;        }
  };
//======================================================================
/** \class ConsoleUi
\brief  Console log output.
*/
//======================================================================
  class ConsoleUi : public LogOut{
      ConsoleUi(const ConsoleUi &obj)             {               }
      ConsoleUi& operator=(const ConsoleUi &obj)  { return *this; }
      typedef std::vector<std::string>    StringVect;
      
    public:
      ConsoleUi();
      virtual ~ConsoleUi();

      void setup(ngl::int2 screenSize, const int conHeight,
                 const char *fontPath, const int fontSize,
                 Console*& console);

                 
      void  render();
      const ngl::Font *font() const { return m_font;  }

      void on_key(const SDL_KeyboardEvent &e);

      virtual void    log(const LogMessage &msg);
      
    private:
      void on_line(const std::string &line);
      uint16_t parse_char(const uint16_t unicode);
      
      ngl::AbstractRenderer*    m_renderer;
      ngl::Font*                m_font;
      StringVect                m_lines;
      ngl::int2                 m_conSize;
      LineInputBuffer           m_inBuffer;
      Console                   *m_console;
  };


  ConsoleUi* init_console_ui(ngl::int2 screenSize, const int conHeight,
                             const char* fontPath, const int fontSize,
                             Console*& console);


//   ConsoleUi* init_gl_console(const int           screenWidth,
//                                 const int           screenHeight,
//                                 const int           conWidth,
//                                 const int           conHeight,
//                                 const char          *fontPath,
//                                 const std::size_t   fontSize,
//                                 Console             *&console);
}



#include "Console.hpp"
#include "FontRenderers.hpp"
#include "FontFace.hpp"
#include <fd/delegate/bind.hpp>

namespace novo{
  //------------------------------------------------------------------//
  LineInputBuffer::LineInputBuffer(){
  }
  //------------------------------------------------------------------//
  LineInputBuffer::~LineInputBuffer(){
  }
  //------------------------------------------------------------------//
  void LineInputBuffer::consume(const CharCode &code){
    if(code.type & CharCode::Ascii){
      m_line+=code.ch;
      if( code.ch == '\n' || code.ch=='\r'){
        if( !m_onLine.empty() )
          m_onLine(m_line);
        m_line.clear();
      }
    }
  }
  //------------------------------------------------------------------//
  void LineInputBuffer::set_callback(Callback callback){
    m_onLine=callback;
  }

  
  struct CharCode{
    enum Type{
      Text    =0x01,
      Symbol  =0x02,
      Cmd     =0x04,
      Ascii   =Text | Symbol
    };
    union{
      uint16_t    code;
      struct{
        uint8_t type;
        char    ch;
      };
    };
    
    CharCode(const uint16_t _code)               :code(_code)         {}
    CharCode(const uint8_t _type, const char _ch):type(_type), ch(_ch){}
    operator uint16_t() const                    { return code;        }
  };
  //------------------------------------------------------------------//
  bool is_one_of(const char ch, const char* set){
    if( !set || !(*set) )
      return false;

    do{
      if(ch == *set)
        return true;
    }while( *(++set) );

    return false;
  }

  
  //------------------------------------------------------------------//
  inline ConsoleUi::ConsoleUi()
  :LogOut(new ConsoleFormatter()),
  m_font(0){
    m_inBuffer.set_callback( fd::bind(&ConsoleUi::on_line, this) );
  }
  //------------------------------------------------------------------//
  inline ConsoleUi::~ConsoleUi(){
    if( m_font )
      delete m_font;
    if( m_renderer )
      delete m_renderer;
  }
  //------------------------------------------------------------------//
  inline void ConsoleUi::setup(ngl::int2 screenSize, const int height,
                               const char *fontPath, const int fontSize,
                               Console*& console){
    m_renderer  = ngl::create_renderer(ngl::Renderer::VBO);
    m_font      = new ngl::Font(fontPath, fontSize);
    m_font->init_position( screenSize.y );
    m_conSize.set(screenSize.width, height);
    
    if( !(console) )
      console = new Console();
    m_console = console;
  }
  //------------------------------------------------------------------//
  inline void ConsoleUi::render(){
    if( !m_font )
      return;

    size_t maxLines = m_conSize.height 
                      / m_font->face()->maxSize().height;
    maxLines = maxLines > m_lines.size() ? m_lines.size() : maxLines;
    for(size_t i = m_lines.size() - maxLines; i < m_lines.size(); ++i){
      m_font->cprint( m_lines[i] );
    }
//  typedef StringVect::const_iterator StringVectIterator;
//  for( StringVectIterator i=m_lines.begin(); i != m_lines.end(); ++i ){
//    m_font->cprint( *i );
//  }
    m_font->update_cache();

    m_renderer->render( *m_font );
  }
  //------------------------------------------------------------------//
  inline void ConsoleUi::on_key(const SDL_KeyboardEvent &e){
    if( e.type == SDL_KEYDOWN ){
      const uint16_t unicode=e.keysym.unicode;
      m_inBuffer.consume( parse_char(e.keysym.unicode) );
    }
  }
  //------------------------------------------------------------------//
  inline void ConsoleUi::log(const LogMessage &msg){
    m_lines.push_back( (const char*)formatter()->format_message(msg) );
  }
  //------------------------------------------------------------------//
  inline void ConsoleUi::on_line(const std::string &line){
    m_console->process_input(line);
  }
  //------------------------------------------------------------------//
  inline uint16_t ConsoleUi::parse_char(const uint16_t unicode){
    const char ch       = unicode & 0x7F;
    const bool isLower  = ch >= 'a' && ch <= 'z';
    const bool isUpper  = ch >= 'A' && ch <= 'Z';
    const bool isDigit  = ch >= '0' && ch <= '9';
    const bool isWhite  = is_one_of(ch, "\n\t\r ");
    const bool isText   = isLower || isUpper || isDigit || isWhite;
    const bool isSymbol = is_one_of(ch, 
                                    "`~!@#$%^&*()_+{[}]|\\:;\"'<,>.?/");

    if( isSymbol )        return CharCode( CharCode::Symbol, ch );
    else if( isText )     return CharCode( CharCode::Text,   ch );
    else                  return CharCode( CharCode::Cmd,    ch );
  }
  





  
  //------------------------------------------------------------------//
  inline ConsoleUi* init_console_ui(ngl::int2   screenSize,
                                    const int   conHeight,
                                    const char* fontPath, 
                                    const int   fontSize,
                                    Console*&   console){
    ngl::init_extensions();
    ngl::freetype::init();
    
    ConsoleUi *out=new ConsoleUi();
    out->setup( screenSize, conHeight, fontPath, fontSize, console );

    logger()->add_output( out, LogLevel::App | LogLevel::Warning );

    return out;
  }
}

#endif/* __NOVO_CONSOLETMP_HPP__ */
