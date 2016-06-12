#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include "core_yash.h"
#include <sys/types.h>

namespace cy
{
  class TCPClient
  {
  private:
    int _fd;
    int _port;

    void* _server;
    void* _server_address;

  public:

    TCPClient();
    TCPClient(const char* address);
    TCPClient(const char* ipaddr, int port);
    TCPClient(long ipaddr, int port);

    bool setConnectionIP(const char* address);
    bool setConnectionIP(long ipaddr);
    bool setConnectionPort(int port);

    int connect();

  private:

    ssize_t write_int(void* dptr, size_t len, int flags);
    ssize_t read_int(void* dptr, size_t len, int flags);

  public:

    ssize_t write(void* dptr, size_t len);
    ssize_t write(const char* str);
    ssize_t write(char* str);
    ssize_t write(char ch);

    ssize_t read(void* buffer, size_t buflen);
    ssize_t read(char& ch);
    ssize_t peek(void* buffer, size_t buflen);
    ssize_t peek(char& ch);
  };
}

#endif // TCPCLIENT_H
