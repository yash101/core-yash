#include "tcpserver.h"
#include <netinet/in.h>

cy::TCPConnection::TCPConnection()
{
  this->_addrinf = NULL;
  this->_tv = NULL;
  this->_addrinf = new struct sockaddr_in;
  this->_tv = new struct timeval;
}

cy::TCPConnection::~TCPConnection()
{
  if(this->_addrinf != NULL)
    delete ((struct sockaddr_in*) this->_addrinf);

  if(this->_tv != NULL)
    delete ((struct timeval*) this->_tv);
}
