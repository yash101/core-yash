#include "tcpserver.h"
#include "cyexception.h"
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

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



//Write data to the socket connection
ssize_t cy::TCPConnection::write(void* dptr, size_t len)
{
  while(true)
  {
    ssize_t ret = send(this->_fd, dptr, len, MSG_NOSIGNAL);
    if(ret < (ssize_t) len || ret < 0)
    {
      //We should continue to try to resend
      if(errno == EINTR  || errno == ENOBUFS || errno == ENOMEM)
      {
        continue;
      }

      if(errno == EACCES || errno == EBADF || errno == ECONNRESET || errno == EDESTADDRREQ || errno == EFAULT || errno == EINVAL || errno == EMSGSIZE || errno == ENOTCONN || errno == ENOTSOCK || errno == EOPNOTSUPP || errno == EPIPE)
      {
        throw CYEXCEPTION("There is an issue with the socket or the connection. Thus, it is important that we end this connection!", errno);
      }
    }

    return ret;
  }
}

//Write data to the socket connection
ssize_t cy::TCPConnection::write(const char* str)
{
  ssize_t len = strlen(str);
  while(true)
  {
    ssize_t ret = send(this->_fd, str, len, MSG_NOSIGNAL);
    if(ret < len || ret < 0)
    {
      //We should continue to try to resend
      if(errno == EINTR  || errno == ENOBUFS || errno == ENOMEM)
      {
        continue;
      }

      if(errno == EACCES || errno == EBADF || errno == ECONNRESET || errno == EDESTADDRREQ || errno == EFAULT || errno == EINVAL || errno == EMSGSIZE || errno == ENOTCONN || errno == ENOTSOCK || errno == EOPNOTSUPP || errno == EPIPE)
      {
        throw CYEXCEPTION("There is an issue with the socket or the connection. Thus, it is important that we end this connection!", errno);
      }
    }

    return ret;
  }
}

//Write data to the socket connection
ssize_t cy::TCPConnection::write(char* str)
{
  size_t len = strlen(str);
  while(true)
  {
    ssize_t ret = send(this->_fd, str, len, MSG_NOSIGNAL);
    if(ret < (ssize_t) len || ret < 0)
    {
      //We should continue to try to resend
      if(errno == EINTR  || errno == ENOBUFS || errno == ENOMEM)
      {
        continue;
      }

      if(errno == EACCES || errno == EBADF || errno == ECONNRESET || errno == EDESTADDRREQ || errno == EFAULT || errno == EINVAL || errno == EMSGSIZE || errno == ENOTCONN || errno == ENOTSOCK || errno == EOPNOTSUPP || errno == EPIPE)
      {
        throw CYEXCEPTION("There is an issue with the socket or the connection. Thus, it is important that we end this connection!", errno);
      }
    }

    return ret;
  }
}

//Write data to the socket connection
ssize_t cy::TCPConnection::write(char ch)
{
  while(true)
  {
    ssize_t ret = send(this->_fd, &ch, sizeof(char), MSG_NOSIGNAL);
    if(ret < (ssize_t) sizeof(char) || ret < 0)
    {
      //We should continue to try to resend
      if(errno == EINTR  || errno == ENOBUFS || errno == ENOMEM)
      {
        continue;
      }

      if(errno == EACCES || errno == EBADF || errno == ECONNRESET || errno == EDESTADDRREQ || errno == EFAULT || errno == EINVAL || errno == EMSGSIZE || errno == ENOTCONN || errno == ENOTSOCK || errno == EOPNOTSUPP || errno == EPIPE)
      {
        throw CYEXCEPTION("There is an issue with the socket or the connection. Thus, it is important that we end this connection!", errno);
      }
    }

    return ret;
  }
}

ssize_t cy::TCPConnection::read_int(void* buffer, size_t len, int flags)
{
  while(true)
  {
    ssize_t ret = recv(this->_fd, buffer, len, flags);
    if(ret <= (ssize_t) len || ret < 0)
    {
      if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR || errno == ENOMEM)
        continue;
      else if(errno == EBADF || errno == ECONNREFUSED || errno == EFAULT || errno == EINVAL || errno == ENOTCONN || errno == ENOTSOCK)
        throw CYEXCEPTION("There is an issue with the socket or the connection. Thus, it is important that we end this connection!", errno);
    }

    return ret;
  }
}

ssize_t cy::TCPConnection::read(void* buffer, size_t buflen)
{
  return this->read_int(buffer, buflen, 0);
}

ssize_t cy::TCPConnection::read(char& ch)
{
  return this->read_int(&ch, sizeof(char), 0);
}

ssize_t cy::TCPConnection::peek(void* buffer, size_t len)
{
  return this->read_int(buffer, len, MSG_PEEK);
}

ssize_t cy::TCPConnection::peek(char& ch)
{
  return this->read_int(&ch, sizeof(char), 0);
}
