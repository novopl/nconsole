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

#define _B(METHOD)  std::bind(&METHOD, this, _1)

namespace novo{
  const int kConsoleMsg = LogMsg::User;

  //------------------------------------------------------------------//
  void tokenize(CArgs *out, const String &in, 
                const char delim=' ', const char group='\"'){
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
    CArgs tokens;
    tokenize( &tokens, input, ' ', '\"' );

    if( tokens.empty() )
      return;
    
    cprint("] %s\n", input.c_str());

    Command *c = find( tokens[0] );
    if( !c ){
      logw( "Command '%s' not found.\n", tokens[0].c_str() );
      return;
    }

    c->func(tokens);
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
  void Console::cmd_cmdlist(const CArgs &args){
    Commands alphaSorted(m_commands);
    std::sort( alphaSorted.begin(), alphaSorted.end() );

    for( const auto &cmd: alphaSorted){
      cprint( " %-20s -- %s\n", cmd.name.c_str(), cmd.desc.c_str() );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_cvarlist(const CArgs &args){
    Commands alphaSorted(m_commands);
    std::sort( alphaSorted.begin(), alphaSorted.end() );

    for( const auto &cmd: alphaSorted){
      cprint( " %-20s -- %s\n", cmd.name.c_str(), cmd.desc.c_str() );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_cvar(const CArgs &args){
  }
  //------------------------------------------------------------------//
  void Console::cmd_help(const CArgs &args){
    if( args.size() == 1 ){
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
    else if( args.size() == 2 ){
      // Print info about requested command
      Command *c = find( args[1] );
      cprint("%s\n", c->name.c_str());
      cprint("  %s\n", c->desc.c_str());
    }
    else{
      logw("Too many arguments\n");
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_echo(const CArgs &args){
    if(args.empty()){
      cprint("\n");
      return;
    }

    String out;
    typedef CArgs::const_iterator  Iter;
    for(Iter it = args.begin() + 1; it != args.end(); ++it){
      out+=*it+" ";
    }
    cprint("%s\n", out.c_str());
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

