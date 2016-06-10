#include "tcpserver.h"
#include <netinet/in.h>
#include <unistd.h>

cy::TCPConnection::TCPConnection()
{
  this->_addrinf = NULL;
  this->_tv = NULL;
  this->_addrinf = NULL;
  this->_addrinf = new struct sockaddr_in;
  this->_tv = new struct timeval;
}

cy::TCPConnection::~TCPConnection()
{
#ifdef _WIN32
  shutdown(this->_fd, 1);
  closesocket(this->_fd);
#else
  shutdown(this->_fd, SHUT_RDWR);
  close(this->_fd);
#endif

  if(this->_addrinf != NULL)
//    delete this->_addrinf;
//    delete ((struct sockaddr_in*) this->_addrinf);

  if(this->_tv != NULL)
    delete ((struct timeval*) this->_tv);
}

int cy::TCPConnection::write(void* data, size_t len)
{
  return send(this->_fd, data, len, MSG_NOSIGNAL);
}

int cy::TCPConnection::read(void* arr, size_t len)
{
  return recv(this->_fd, arr, len, MSG_NOSIGNAL);
}
