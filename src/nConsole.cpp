//======================================================================
/**
\file            Console.cpp
\author          Mateusz 'novo' Klos
\date            July 20, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#include "nConsole.hpp"
#include <fd/delegate/bind.hpp>
#include <nLogger.hpp>

#define _B(METHOD)  fd::bind(&METHOD, this)

namespace novo{
  //------------------------------------------------------------------//
  void tokenize(StringVector *out, const String &in, 
                const char delim=' ', const char group='\"'){
    out->clear();

    typedef String::size_type Index_t;
    Index_t off = 0;

    for(Index_t i = 0; i < in.length(); ++i){
      if( in[i] == delim ){
        if( i > off+1 ){
          out->push_back(in.substr(off, i - off));
        }
        off = i + 1;
      }
      else if( in[i] == group ){
      }
    }
    if( off < in.length() )
      out->push_back( in.substr(off) );
  }


  struct Console::Command{
    CommandFunc   func;
    String        name;
    String        desc;
  };
  //------------------------------------------------------------------//
  Console::Console(const Console &obj){
  }
  //------------------------------------------------------------------//
  Console& Console::operator=(const Console &obj){
    return *this;
  }
  //------------------------------------------------------------------//
  Console::Console(){
    logger()->set_tag(LogMsg::App, "game");
    app_log("Initializing console.\n");
    add( _B(Console::help),    "help",     "Print this help" );
    add( _B(Console::echo),    "echo",     "Echo reply" );
    add( _B(Console::cmdlist), "cmdlist",  "Command list." );
  }
  //------------------------------------------------------------------//
  Console::~Console(){
    app_log("Shutting down console.\n");
    remove("help");
    remove("echo");
    remove("cmdlist");

    typedef Commands::const_iterator    Iter;
    for(Iter  it = m_commands.begin(); it != m_commands.end(); ++it){
      log(LogMsg::Warning, "Command '%s' wasn't removed.\n", 
          it->name.c_str());
    }
  }
  //------------------------------------------------------------------//
  void Console::process_input(const String &input){
    StringVector tokens;
    tokenize( &tokens, input, ' ', '\"' );

    if( tokens.empty() )
      return;
    
    log("] %s\n", input.c_str());

    Command *c = find( tokens[0] );
    if( !c ){
      logw( "Command '%s' not found.\n", tokens[0].c_str() );
      return;
    }

    c->func(tokens);
  }
  //------------------------------------------------------------------//
  bool Console::add(CommandFunc cmd, const String &name, 
                    const String &description){
    if( !cmd || name.empty() || description.empty() || exists(name) )
      return false;

    Command c;
    c.func  = cmd;
    c.name  = name;
    c.desc  = description;

    m_commands.push_back(c);
    return true;
  }
  //------------------------------------------------------------------//
  bool Console::remove(const String &name){
    typedef Commands::iterator Iter;
    for(Iter it = m_commands.begin(); it != m_commands.end(); ++it){
      if( it->name == name ){
        m_commands.erase(it);
        return true;
      }
    }
    return false;
  }
  //------------------------------------------------------------------//
  bool Console::exists(const String &name){
    return find(name) != 0;
  }
  //------------------------------------------------------------------//
  Console::Command* Console::find(const String &name){
    typedef Commands::iterator  Iter;
    for(Iter it = m_commands.begin(); it != m_commands.end(); ++it){
      if( it->name == name ){
        return &*it;
      }
    }
    return 0;
  }
  //------------------------------------------------------------------//
  void Console::echo(const StringVector &args){
    if(args.empty()){
      log("\n");
      return;
    }

    String out;
    typedef StringVector::const_iterator  Iter;
    for(Iter it = args.begin() + 1; it != args.end(); ++it){
      out+=*it+" ";
    }
    log("%s\n", out.c_str());
  }
  //------------------------------------------------------------------//
  void Console::help(const StringVector &args){
  }
  //------------------------------------------------------------------//
  void Console::cmdlist(const StringVector &args){
  }
}

