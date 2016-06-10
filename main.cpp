#include "core_yash.h"
#include "tcpserver.h"

int main()
{
  cy::TCPServer server;
  server.setPort(2048);
  server.startServer();
  return 0;
}
