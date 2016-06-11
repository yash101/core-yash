#include "tcpserver.h"
#include <netinet/in.h>
#include <unistd.h>

cy::TCPConnection::TCPConnection() :
  _fd(-1)
{
  this->_addrinf = NULL;
  this->_tv = NULL;
  this->_addrinf = NULL;
  this->_addrinf = new struct sockaddr_in;
  this->_tv = new struct timeval;
}

cy::TCPConnection::~TCPConnection()
{
  //Try to end the socket if there was one in the first place
  if(_fd >= 0)
  {
#ifdef _WIN32
    shutdown(this->_fd, 1);
    closesocket(this->_fd);
#else
    shutdown(this->_fd, SHUT_RDWR);
    close(this->_fd);
#endif
  }

  //GC
  if(this->_addrinf != NULL)
    delete ((struct sockaddr_in*) this->_addrinf);

  //...Repeat...
  if(this->_tv != NULL)
    delete ((struct timeval*) this->_tv);


  //Update the server object's connected client count
  this->server->_connectedClientsMutex.lock();
  this->server->_connectedClients--;
  this->server->_connectedClientsMutex.unlock();
}

int cy::TCPConnection::write(void* data, size_t len)
{
  return send(this->_fd, data, len, MSG_NOSIGNAL);
}

int cy::TCPConnection::read(void* arr, size_t len)
{
  return recv(this->_fd, arr, len, MSG_NOSIGNAL);
}
