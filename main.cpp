#include "core_yash.h"
#include "tcpserver.h"

int main()
{
  cy::TCPServer server;
  server.setPort(2048);
  server.setTimeout(250000, 0);
  server.setTimeout(0, 3);
  server.setMaxConnectedClients(3);
  server.startServer();
  return 0;
}
