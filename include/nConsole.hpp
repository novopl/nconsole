//======================================================================
/**
\file            Console.hpp
\author          Mateusz 'novo' Klos
\date            July 20, 2010



Copyright (c) 2010 Mateusz 'novo' Klos
*/
//======================================================================
#if !defined(__NOVO_CONSOLE_HPP__)
#define __NOVO_CONSOLE_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace novo{
  typedef std::string           String;
  typedef std::vector<String>   CArgs;

  //====================================================================
  /** 
    Console class.
  */
  class Console{
    Console(const Console &obj)             = delete;
    Console(const Console &&obj)            = delete;
    Console& operator=(const Console &obj)  = delete;
    Console& operator=(const Console &&obj) = delete;

    typedef std::function<void(const String &)>  CmdFunc;
    
  public:
    Console();
    virtual ~Console();

    void process_input(const String &input);
    bool add(const String &name, const String &desc, CmdFunc cmd);
    bool remove(const String &name);
    bool exists(const String &name);

    void    set(const String &name, const String &value);
    String  get(const String &name);
    float   get_float(const String &name);
    int     get_int(const String &name);

    void cmd_cmdlist(const String &args);
    void cmd_cvarlist(const String &args);
    void cmd_cvar(const String &args);
    void cmd_echo(const String &args);
    void cmd_help(const String &args);

    void tokenize(CArgs *out, const String &in,
                  const char delim=' ', const char group='\"');

  private:
    struct Command;
    struct Cvar;
    typedef std::vector<Command>    Commands;
    typedef std::vector<Cvar>       Cvars;
    
    Command* find(const String &name);

    Commands  m_commands;
    Cvars     m_cvars;
  };

  extern const int kConsoleMsg;
  extern void cprint(const char *fmt, ...);
}

#endif/* __NOVO_CONSOLE_HPP__ */
