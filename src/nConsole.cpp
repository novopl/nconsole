//======================================================================
/**
\file            Console.cpp
\author          Mateusz 'novo' Klos
\date            July 20, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#include "nConsole.hpp"
#include <nLogger.hpp>
#include <algorithm>
#include <cstdlib>

#define _B(METHOD)  std::bind(&METHOD, this, _1)

namespace novo{
  const int kConsoleMsg = LogMsg::User;

  //------------------------------------------------------------------//
  void Console::tokenize(CArgs *out, const String &in, 
                         const char delim, const char group){
    out->clear();

    typedef String::size_type Index_t;
    Index_t off = 0;
    bool ingrp  = false;

    for(Index_t i = 0; i < in.length(); ++i){
      if( in[i] == delim && !ingrp ){
        if( i > off+1 ){
          out->push_back(in.substr(off, i - off));
        }
        off = i + 1;
      }
      else if( in[i] == group ){
        ingrp = !ingrp;
      }
    }
    if( off < in.length() )
      out->push_back( in.substr(off) );
  }


  //====================================================================
  struct Console::Command{
    CmdFunc   func;
    String    name;
    String    desc;

    bool operator<(const Command &cmd) const{
      return name < cmd.name;
    }
  };
  //====================================================================
  struct Console::Cvar{
    String  name;
    String  desc;
    String  value;

    bool operator<(const Cvar &cvar) const{
      return name < cvar.name;
    }
  };
  //------------------------------------------------------------------//
  Console::Console(){
    logger()->set_tag(kConsoleMsg, "console");
    cprint("Initializing console.\n");

    using namespace std::placeholders;
    add( "cmdlist", "List of available commands", 
        std::bind( &Console::cmd_cmdlist, this, _1 ));
    add( "cvarlist", "List of all registered cvars", 
        _B( Console::cmd_cvarlist ) );
    add( "cvar", "Set/get cvar value", 
        _B( Console::cmd_cvar ) );
    add( "help", "Print this help", 
        _B( Console::cmd_help ) );
    add( "echo", "Print message to the console",   
        _B( Console::cmd_echo ) );
  }
  //------------------------------------------------------------------//
  Console::~Console(){
    cprint("Shutting down console.\n");
    remove("cmdlist");
    remove("cvarlist");
    remove("cvar");
    remove("help");
    remove("echo");

    for( const auto &cmd: m_commands ){
      logw("Command '%s' wasn't removed.\n", cmd.name.c_str());
    }
  }
  //------------------------------------------------------------------//
  void Console::process_input(const String &input){
    if( input.empty() )
      return;

    auto  off   = input.find(" ");
    String name = input.substr(0, off);
    String args = off != String::npos ? input.substr(off+1) : "";

    cprint("] %s\n", input.c_str());

    Command *c = find( name );
    if( c )
      c->func( args );
    else
      logw( "Command '%s' not found.\n", name.c_str() );

    //cprint("] %s\n", input.c_str());

    //CArgs tokens;
    //tokenize( &tokens, input, ' ', '\"' );

    //if( tokens.empty() )
    //  return;
    //
    //cprint("] %s\n", input.c_str());

    //Command *c = find( tokens[0] );
    //if( !c ){
    //  logw( "Command '%s' not found.\n", tokens[0].c_str() );
    //  return;
    //}

    //c->func(tokens);
  }
  //------------------------------------------------------------------//
  bool Console::add(const String &name, const String &desc, CmdFunc fn){
    if( !fn || name.empty() || desc.empty() || exists(name) )
      return false;

    Command cmd;
    cmd.func  = fn;
    cmd.name  = name;
    cmd.desc  = desc;

    m_commands.push_back( cmd );
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
    for( auto &cmd: m_commands ){
      if( cmd.name == name )
        return &cmd;
    }
    return nullptr;
  }
  //------------------------------------------------------------------//
  void Console::set(const String &name, const String &value){
    auto it = std::find_if( m_cvars.begin(), m_cvars.end(),
                            [&](const Cvar &c){
      return c.name == name; 
    });
    if( it != m_cvars.end() ){
      it->value = value;
    }
    else{
      logw( "Setting non-existing cvar: %s", name.c_str() );
      m_cvars.push_back( Cvar{ name, "", value } );
    }
  }
  //------------------------------------------------------------------//
  String Console::get(const String &name){
    auto it = std::find_if( m_cvars.begin(), m_cvars.end(),
                            [&](const Cvar &c){
      return c.name == name; 
    });
    return it != m_cvars.end() ? it->value : "";
  }
  //------------------------------------------------------------------//
  float Console::get_float(const String &name){
    auto value = get( name );
    return atof( value.c_str() );
  }
  //------------------------------------------------------------------//
  int Console::get_int(const String &name){
    auto value = get( name );
    return atoi( value.c_str() );
  }
  //------------------------------------------------------------------//
  void Console::cmd_cmdlist(const String &args){
    Commands alphaSorted(m_commands);
    std::sort( alphaSorted.begin(), alphaSorted.end() );

    for( const auto &cmd: alphaSorted){
      cprint( " %-20s -- %s\n", cmd.name.c_str(), cmd.desc.c_str() );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_cvarlist(const String &args){
    Cvars alphaSorted(m_cvars);
    std::sort( alphaSorted.begin(), alphaSorted.end() );

    for( const auto &cmd: alphaSorted){
      cprint( " %-20s %10s -- %s\n", 
              cmd.name.c_str(), cmd.value.c_str(), cmd.desc.c_str() );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_cvar(const String &args){
    CArgs cargs;
    tokenize( &cargs, args );

    if( cargs.size() == 1 ){
      String val = get( cargs[0] );
      cprint( " %s = %s\n", cargs[0].c_str(), val.c_str() );
    }
    else if( cargs.size() == 2 ){
      set( cargs[0], cargs[1] );
      cprint( " %s = %s\n", cargs[0].c_str(), cargs[1].c_str() );
    }
    else{
      logw("ERROR: Too many arguments!");
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_help(const String &args){
    if( args.empty() ){
      // Print general help.
      cprint("To execute a command use syntax:\n");
      cprint(" COMMAND [arguments]\n\n");

      cprint("Not all commands accept arguments. Each word is a\n");
      cprint("separate argument. If you want to pass multiple words\n");
      cprint("as one argument, enclose them in double quotation\n");
      cprint("marks - '\"'\n\n");

      cprint("To see the list of all available commands type:\n");
      cprint("cmdlist\n\n");

      cprint("To get the help about a specific command type:\n");
      cprint("help COMMAND_NAME\n");
    }
    else{
      CArgs cargs;
      tokenize( &cargs, args );
      
      for( const auto &arg: cargs ){
        Command *c = find( cargs[0] );
        cprint("%s\n", c->name.c_str());
        cprint("  %s\n", c->desc.c_str());
      }
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_echo(const String &args){
    cprint("%s\n", args.c_str());
  }


  //------------------------------------------------------------------//
  void cprint(const char *fmt, ...){
    va_list argList;
    va_start(argList, fmt);

    char *msg = log_format(fmt, argList);
    logger()->log( kConsoleMsg, msg );
    delete[] msg;
  }

}

