#include "tcpserver.h"
#include "cyexception.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <Windows.h>

static WSADATA WsaData;
static WORD WsaVersionRequested = NULL;
static int WsaError = 0;
static bool WsaInitialized = false;

#else

#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#endif

namespace thread_helper
{
  struct threader
  {
    void (*function)(void* data, void* data2);
    void* data;
    void* data2;
  };

  void* threader_func(void* data)
  {
    ((threader*) data)->function(((threader*) data)->data, ((threader*) data)->data2);
    return NULL;
  }
}

cy::TCPServer::TCPServer()
{
#ifdef _WIN32
  if(!WsaInitialized)
  {
    WsaVersionRequested = MAKEWORD(2, 2);
    WsaError = WSAStartup(WsaVersionRequested, &WsaData);

    if(WsaError != 0)
    {
      fprintf(stderr, "Unable to find the Windows Socket (WinSock 2) Library! Fatal!");
      throw CYEXCEPTION("Unable to find the Windows Socket (WinSock 2) Library! Fatal!", WsaError);
    }
  }
#endif
  this->initVars();

  this->_addrinf = (void*) new struct sockaddr_in;
  this->_timeoutStruct = (void*) new struct timeval;

  memset(this->_addrinf, 0, sizeof(struct sockaddr_in));
  memset(this->_timeoutStruct, 0, sizeof(struct timeval));
}

cy::TCPServer::~TCPServer()
{
  if(this->_addrinf != NULL)
  {
    delete ((struct sockaddr_in*) this->_addrinf);
  }

  if(this->_timeoutStruct != NULL)
  {
    delete ((struct sockaddr_in*) this->_timeoutStruct);
  }
}

bool cy::TCPServer::setMaxConnectedClients(int max)
{
  this->_maxConnectedClientsMutex.lock();
  this->_maxConnectedClients = max;
  this->_maxConnectedClientsMutex.unlock();

  return true;
}

int cy::TCPServer::getMaxConnectedClients()
{
  this->_maxConnectedClientsMutex.lock();
  int ret = _maxConnectedClients;
  this->_maxConnectedClientsMutex.unlock();
  return ret;
}

int cy::TCPServer::getConnectedClientsCount()
{
  this->_connectedClientsMutex.lock();
  int clict = this->_connectedClients;
  this->_connectedClientsMutex.unlock();
  return clict;
}

bool cy::TCPServer::setPort(int16_t portno)
{
  if(this->_isRunning)
    return false;
  this->_port = portno;
  return true;
}

int16_t cy::TCPServer::getPort()
{
  return this->_port;
}

bool cy::TCPServer::setConnectionQueueSize(int qs)
{
  if(this->_isRunning)
    return false;
  this->_connectionQueueSize = qs;
  return true;
}

int cy::TCPServer::getConnectionQueueSize()
{
  return this->_connectionQueueSize;
}

bool cy::TCPServer::setTimeoutUsec(int usec)
{
  ((struct timeval*) this->_timeoutStruct)->tv_usec = usec;
  return true;
}

bool cy::TCPServer::setTimeoutSec(int sec)
{
  ((struct timeval*) this->_timeoutStruct)->tv_sec = sec;
  return true;
}

bool cy::TCPServer::setTimeout(int usec, int sec)
{
  ((struct timeval*) this->_timeoutStruct)->tv_sec = sec;
  ((struct timeval*) this->_timeoutStruct)->tv_usec = usec;
  return true;
}

int cy::TCPServer::getTimeoutUsec()
{
  return ((struct timeval*) this->_timeoutStruct)->tv_usec;
}

int cy::TCPServer::getTimeoutSec()
{
  return ((struct timeval*) this->_timeoutStruct)->tv_sec;
}

bool cy::TCPServer::isServerRunning()
{
  return this->_isRunning;
}

bool cy::TCPServer::startServer()
{
  if(this->_isRunning)
    throw CYEXCEPTION("Cannot start server! Server is already running!", 0);

  //Check the make sure that we allocated memory for the address structure
  if(this->_addrinf == NULL)
    throw CYEXCEPTION("Address structure [POSIX] not allocated; Cannot continue;", 0);



  //Create the socket and check for success
  this->_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(this->_fd < 0)
    throw CYEXCEPTION("Unable to create socket!", this->_fd);



  //Flag that the server is running
  this->_isRunning = true;



  //Fill out the fields of the (struct sockaddr_in), so we can bind to the socket
  ((struct sockaddr_in*) this->_addrinf)->sin_family = AF_INET;
  ((struct sockaddr_in*) this->_addrinf)->sin_addr.s_addr = INADDR_ANY;
  ((struct sockaddr_in*) this->_addrinf)->sin_port = htons(this->_port);



  //Set SO_REUSEADDR to 1 to allow restarting the server with current active/TIME_WAIT connections
  int reuseAddress = 1;
  //Ugliness caused by some platform-based shit
#ifdef _WIN32
  setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuseAddress, sizeof(int));
#else
  setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (const void*) &reuseAddress, sizeof(int));
#endif



  //Bind to the socket
  int ret = bind(this->_fd, (struct sockaddr*) this->_addrinf, sizeof(struct sockaddr_in));
  if(ret < 0)
    throw CYEXCEPTION("Unable to bind to socket", ret);



  //Run the callback: When the port/IP has been assigned
#ifdef _WIN32
  typedef int socklen_t;
#endif

  struct sockaddr_in sin;
  socklen_t len = sizeof(struct sockaddr_in);
  if(getsockname(this->_fd, (struct sockaddr*) &sin, (socklen_t*) &len) < 0)
    fprintf(stderr, "Unable to gather information from socket serving from!");
  else
  {
    this->_port = ntohs(sin.sin_port);
  }
  this->whenPortIsAssigned();


  this->listeningThread();

  return false;
}

void cy::TCPServer::listeningThread()
{
  //Begin listening (for incoming connections)
  listen(this->_fd, this->_connectionQueueSize);


  //Used by accept; IDK why it needs a pointer!
  //Just an int; Use stack; No point in using heap and
  //possible having a memory leak
  int ss = sizeof(struct sockaddr_in);



  while(true)
  {
    //Allocate space for a cy::TCPConnection object
    cy::TCPConnection* connection = NULL;
    try
    {
      connection = new cy::TCPConnection;
    }
    catch(std::bad_alloc& e)
    {
      continue;
    }



    //[ENFORCE] Connected Client Quota
    while(true)
    {
      size_t nmc = 0;
      size_t nc = 0;

      this->_maxConnectedClientsMutex.lock();
      nmc = this->_maxConnectedClients;
      this->_maxConnectedClientsMutex.unlock();

      if(nmc == 0) break;

      this->_connectedClientsMutex.lock();
      nc = this->_connectedClients;
      this->_connectedClientsMutex.unlock();

      if(nc <= nmc) break;
    }



    //Accept a new connection
    connection->_fd = accept(
      this->_fd,
      (struct sockaddr*) connection->_addrinf,
      (socklen_t*) &ss
    );



    //We failed to accept the connection for whatever reason
    if(connection->_fd < 0)
    {
      delete connection;
      continue;
    }



    //Match to the master struct timeval*
    ((struct timeval*) connection->_tv)->tv_sec = ((struct timeval*) this->_timeoutStruct)->tv_sec;
    ((struct timeval*) connection->_tv)->tv_usec = ((struct timeval*) this->_timeoutStruct)->tv_usec;



    //Apply the timeout
    if(setsockopt(
      connection->_fd,
      SOL_SOCKET,
      SO_RCVTIMEO,
      connection->_tv,
      sizeof(struct timeval)
    ))
    {
      delete connection;
      continue;
    }


    //Launch the new thread!
  }
}

void cy::TCPServer::whenPortIsAssigned()
{
}

void cy::TCPServer::worker(TCPConnection *connection)
{
}

//+----------------------------+//
//|****************************|//
//|****NEEDS IMPLEMENTATION****|//
//|****************************|//
//+----------------------------+//

bool cy::TCPServer::stopServer()
{
  return false;
}

void cy::TCPServer::cleanUpActiveConnectionsList()
{

}

std::vector<cy::TCPConnection**> getActiveConnectionsList()
{
  return std::vector<cy::TCPConnection**>();
}
