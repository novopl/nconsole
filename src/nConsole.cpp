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
  using namespace logger;
  using boost::format;

  const int kConsoleMsg = logger::kGame;

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
    String    name;
    String    desc;
    String    value;
    OnChange  onChange;

    bool operator<(const Cvar &cvar) const{
      return name < cvar.name;
    }
  };
  //------------------------------------------------------------------//
  Console::Console(){
    //logger()->set_tag(kConsoleMsg, "console");
    cprint("Initializing console.\n");

    using namespace std::placeholders;
    add( "cmdlist", "List of available commands", 
        std::bind( &Console::cmd_cmdlist, this, _1 ));
    add( "cvarlist", "List of all registered cvars", 
        _B( Console::cmd_cvarlist ) );
    add( "set", "Set the cvar value", 
        _B( Console::cmd_set ) );
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
    remove("set");
    remove("help");
    remove("echo");

    for( const auto &cmd: m_commands ){
      logw(str( format("Command '%1%' wasn't removed\n") %cmd.name ));
    }
  }
  //------------------------------------------------------------------//
  void Console::process_input(const String &input){
    if( input.empty() )
      return;

    auto  off   = input.find(" ");
    String name = input.substr(0, off);
    String args = off != String::npos ? input.substr(off+1) : "";

    cprint( format("] %s\n") %input);

    Command *cmd = find( name );
    if( cmd )
      cmd->func( args );
    else{
      // Try cvars
      Cvar *cvar = find_cvar( name );
      if( cvar ){
        if( args.empty() )
          cprint( format(" %s = %s\n") %name %cvar->value );
        else{
          CArgs arglist;
          tokenize( &arglist, args );
          set( name, arglist[0] );
          cprint( format(" %s = %s\n") %name %cvar->value );
        }
      }
      else
        logw(str( format("'%1%' not found.\n") % name ));
    }
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
  int Console::exists(const String &name){
    if( find(name) != nullptr )
      return 1;
    else if( find_cvar(name) != nullptr )
      return 2;
    return 0;
  }
  //------------------------------------------------------------------//
  void Console::set(const String &name, const String &value){
    Cvar *c = find_cvar( name );
    if( c ){
      c->value = value;
      for( auto fn: c->onChange ){
        fn( name, value );
      }
    }
    else{
      logw(str( format("Setting non-existing cvar: %1%") %name ));
      m_cvars.push_back( Cvar{ name, "", value } );
    }
  }
  //------------------------------------------------------------------//
  String Console::get(const String &name){
    Cvar *c = find_cvar( name );
    return c ? c->value : "";
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
  void Console::on_change(const String &name, OnChangeFunc fn){
    Cvar *cvar = find_cvar( name );
    if( cvar ){
      cvar->onChange.push_back( fn );
    }
    else{
      m_cvars.push_back( Cvar{ name, "", "", { fn }} );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_cmdlist(const String &args){
    Commands alphaSorted(m_commands);
    std::sort( alphaSorted.begin(), alphaSorted.end() );

    for( const auto &cmd: alphaSorted){
      cprint( format(" %-20s -- %s\n") %cmd.name %cmd.desc );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_cvarlist(const String &args){
    Cvars alphaSorted(m_cvars);
    std::sort( alphaSorted.begin(), alphaSorted.end() );

    for( const auto &cmd: alphaSorted){
      cprint( format(" %-20s %10s -- %s\n") 
              %cmd.name %cmd.value %cmd.desc );
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_set(const String &args){
    CArgs cargs;
    tokenize( &cargs, args );

    if( cargs.size() == 1 ){
      String val = get( cargs[0] );
      cprint( format(" %s = %s\n") %cargs[0] %val );
    }
    else if( cargs.size() == 2 ){
      set( cargs[0], cargs[1] );
      cprint( format(" %s = %s\n") %cargs[0] %cargs[1] );
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
      CArgs arglist;
      tokenize( &arglist, args );
      
      Command *cmd;
      Cvar    *cvar;
      for( const auto &arg: arglist ){
        cmd = find( arg );
        if( cmd ){
          cprint( format("%s\n") %cmd->name );
          cprint( format("  %s\n") %cmd->desc );
        }
        else{
          cvar = find_cvar( arg );
          if( cvar ){
            cprint( format("%s = %s\n") %cvar->name %cvar->value );
            cprint( format("  %s\n") %cvar->desc );
          }
        }
      }
    }
  }
  //------------------------------------------------------------------//
  void Console::cmd_echo(const String &args){
    cprint( format("%s\n") %args);
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
  Console::Cvar* Console::find_cvar(const String &name){
    for( auto &cvar: m_cvars ){
      if( cvar.name == name )
        return &cvar;
    }
    return nullptr;
  }
}


namespace novo{
  //------------------------------------------------------------------//
  void cprint( const std::string &msg ){
    logger::log( kConsoleMsg, msg );
  }
  //------------------------------------------------------------------//
  void cprint( const boost::basic_format<char> &fmt ){
    cprint( str(fmt) );
  }
  //------------------------------------------------------------------//
  //void cprint(const char *fmt, ...){
  //  va_list argList;
  //  va_start(argList, fmt);

  //  char *msg = log_format(fmt, argList);
  //  logger::log( kConsoleMsg, msg );
  //  delete[] msg;
  //}

}

