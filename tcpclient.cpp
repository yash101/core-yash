#include "tcpclient.h"
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stddef.h>

#define INIT(port) _fd(-1), _port(port), _server(NULL), _server_address(NULL)
cy::TCPClient::TCPClient() : INIT(0)
{
  _server = new struct hostent;
  _server_address = new struct sockaddr_in;

  memset(_server, 0, sizeof(struct hostent));
  memset(_server_address, 0, sizeof(struct sockaddr_in));
}

cy::TCPClient::TCPClient(const char* address) : INIT(0)
{
  _server = new struct hostent;
  _server_address = new struct sockaddr_in;

  memset(_server, 0, sizeof(struct hostent));
  memset(_server_address, 0, sizeof(struct sockaddr_in));
}

cy::TCPClient::TCPClient(const char* ipaddr, int port) : INIT(port)
{
  _server = new struct hostent;
  _server_address = new struct sockaddr_in;

  memset(_server, 0, sizeof(struct hostent));
  memset(_server_address, 0, sizeof(struct sockaddr_in));
}

cy::TCPClient::TCPClient(long ipaddr, int port) : INIT(port)
{
  _server = new struct hostent;
  _server_address = new struct sockaddr_in;

  memset(_server, 0, sizeof(struct hostent));
  memset(_server_address, 0, sizeof(struct sockaddr_in));
}
