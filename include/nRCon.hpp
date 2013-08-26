#ifndef __NOVO_RCON_HPP__
#define __NOVO_RCON_HPP__
#include <vector>
#include <string>

namespace novo{
  class   Console;
  struct  LogMsg;

  typedef int Socket_t;

  //====================================================================
  class RConClient{
    RConClient(const RConClient&)             = delete;
    RConClient(const RConClient&&)            = delete;
    RConClient& operator=(const RConClient&)  = delete;
    RConClient& operator=(const RConClient&&) = delete;

  public:
    RConClient();
    virtual ~RConClient();

    bool connected() const;

    void connect( const std::string &host, int port );
    void disconnect();
    void exec( const std::string &cmd );
    void update();


  private:
    void process_buffer();

    std::string         m_host;
    int                 m_port;
    Socket_t            m_sock;
    std::vector<char>   m_buff;
  };


  //====================================================================
  class RConServer{
    RConServer(const RConServer&)             = delete;
    RConServer(const RConServer&&)            = delete;
    RConServer& operator=(const RConServer&)  = delete;
    RConServer& operator=(const RConServer&&) = delete;

  public:
    RConServer();
    virtual ~RConServer();

    bool connected() const;

    void bind( Console *console, int port );
    void update();

  private:
    void send( const LogMsg &msg );

    int       m_port;
    Socket_t  m_sockListen;
    Socket_t  m_sockConn;
    Console   *m_console;
  };
}

#endif/* __NOVO_RCON_HPP__ */
