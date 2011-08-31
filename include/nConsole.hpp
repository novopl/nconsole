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
#include "fd/delegate.hpp"

namespace novo{
  typedef std::string           String;
  typedef std::vector<String>   StringVector;
  //====================================================================
  /** \class Console
  \brief  Console class.
  */
  //====================================================================
  class Console{
      Console(const Console &obj);
      Console& operator=(const Console &obj);

      typedef fd::delegate<void(const StringVector &args)>  CommandFunc;
      
    public:
      Console();
      virtual ~Console();

      void process_input(const String &input);
      bool add( CommandFunc cmd, const String &name, 
                const String &description);
      bool remove(const String &name);
      bool exists(const String &name);

      void echo(const StringVector &args);
      void help(const StringVector &args);
      void cmdlist(const StringVector &args);

    private:
      struct Command;
      typedef std::vector<Command>    Commands;
      
      Command* find(const String &name);

      Commands  m_commands;
  };
}

#endif/* __NOVO_CONSOLE_HPP__ */
