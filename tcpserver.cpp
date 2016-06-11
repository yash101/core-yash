#include "tcpserver.h"
#include "cyexception.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <thread>

//Windows specific stuff
#ifdef _WIN32

//Linking including WinSock2 headers
#pragma comment(lib, "Ws2_32.lib")
#include <Windows.h>

//Variables to store the initialization data for WinSock
static WSADATA WsaData;
static WORD WsaVersionRequested = NULL;
static int WsaError = 0;
static bool WsaInitialized = false;

//Non-Windows specific stuff
#else

#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#endif


//TCP Server constructor
//Initializes the core for sockets and variables
cy::TCPServer::TCPServer()
{
  //Initialize and set up Winsock 2 library
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

  //Initialize any variables needing initialization
  this->initVars();

  //Allocate space for the address structure and timeval structure
  //  Throws std::bad_alloc; If a variable has already been allocated,
  //  the destructor will GC it.
  this->_addrinf = (void*) new struct sockaddr_in;
  this->_timeoutStruct = (void*) new struct timeval;

  //Set to zeroes so we don't have any wierd side effects (undefined behaviour)
  memset(this->_addrinf, 0, sizeof(struct sockaddr_in));
  memset(this->_timeoutStruct, 0, sizeof(struct timeval));
}


//Cleans up
cy::TCPServer::~TCPServer()
{
  //Deallocate if not null
  if(this->_addrinf != NULL)
  {
    delete ((struct sockaddr_in*) this->_addrinf);
  }

  //Repeat :)
  if(this->_timeoutStruct != NULL)
  {
    delete ((struct sockaddr_in*) this->_timeoutStruct);
  }

  //To make code less repetitive
#ifdef _WIN32
#define SHUT_RDWR 1
#endif

  //Shutdown the socket, then close it
  if(shutdown(this->_fd, SHUT_RDWR) < 0)
  {
    fprintf(stderr, "Error. Unable to shutdown socket; FD: %d; ERRNO: %d; ERMSG: %s\n", this->_fd, errno, strerror(errno));
  }
  if(close(this->_fd) < 0)
  {
    fprintf(stderr, "Error. Unable to close socket; FD: %d; ERRNO: %d; ERMSG: %s\n", this->_fd, errno, strerror(errno));
  }
}


//Set the quota of number of clients that may connect
bool cy::TCPServer::setMaxConnectedClients(int max)
{
  this->_maxConnectedClientsMutex.lock();
  this->_maxConnectedClients = max;
  this->_maxConnectedClientsMutex.unlock();

  return true;
}


//Retrieve the quota of number of clients that may connect
int cy::TCPServer::getMaxConnectedClients()
{
  this->_maxConnectedClientsMutex.lock();
  int ret = _maxConnectedClients;
  this->_maxConnectedClientsMutex.unlock();
  return ret;
}


//Count the number of clients currently connected
int cy::TCPServer::getConnectedClientsCount()
{
  this->_connectedClientsMutex.lock();
  int clict = this->_connectedClients;
  this->_connectedClientsMutex.unlock();
  return clict;
}


//Try to set the listening port of the server
//Cannot set port if server is running actively
bool cy::TCPServer::setPort(int16_t portno)
{
  if(this->_isRunning)
    return false;
  this->_port = portno;
  return true;
}


//Retrieve the port the server is currently listening on
int16_t cy::TCPServer::getPort()
{
  return this->_port;
}


//Set the connection queue size
//Cannot set connection queue size of server is running actively
bool cy::TCPServer::setConnectionQueueSize(int qs)
{
  if(this->_isRunning)
    return false;
  this->_connectionQueueSize = qs;
  return true;
}


//Retrieve the connection queue size
int cy::TCPServer::getConnectionQueueSize()
{
  return this->_connectionQueueSize;
}


//Set socket timeout, in microseconds
bool cy::TCPServer::setTimeoutUsec(int usec)
{
  ((struct timeval*) this->_timeoutStruct)->tv_usec = usec;
  return true;
}


//Set socket timeout, in seconds
bool cy::TCPServer::setTimeoutSec(int sec)
{
  ((struct timeval*) this->_timeoutStruct)->tv_sec = sec;
  return true;
}


//Set socket timeout, in seconds and microseconds
bool cy::TCPServer::setTimeout(int usec, int sec)
{
  ((struct timeval*) this->_timeoutStruct)->tv_sec = sec;
  ((struct timeval*) this->_timeoutStruct)->tv_usec = usec;
  return true;
}


//Retrieve the socket timeout in microseconds
int cy::TCPServer::getTimeoutUsec()
{
  return ((struct timeval*) this->_timeoutStruct)->tv_usec;
}


//Retrieve the socket timeout in seconds
int cy::TCPServer::getTimeoutSec()
{
  return ((struct timeval*) this->_timeoutStruct)->tv_sec;
}


//Check if the server is already running
bool cy::TCPServer::isServerRunning()
{
  return this->_isRunning;
}


//Stop the server from accepting any new connections
bool cy::TCPServer::stopServer()
{
  shutdown(this->_fd, SHUT_RDWR);
  close(this->_fd);
  return false;
}


//Start the server
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
  //Ugliness caused by some platform-based shit; The definitions allow us to consolidate
  //the calls.
#ifdef _WIN32
#define sso_tp char*
#else
#define sso_tp const void*
#endif
  if(setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (sso_tp) &reuseAddress, sizeof(int)) < 0)
  {
    this->_isRunning = false;
    CYEXCEPTION("Unable to enable SO_REUSEADDR on the socket!", this->_fd);
  }



  //Bind to the socket
  int ret = bind(this->_fd, (struct sockaddr*) this->_addrinf, sizeof(struct sockaddr_in));
  if(ret < 0)
  {
    this->_isRunning = false;
    throw CYEXCEPTION("Unable to bind to socket", ret);
  }



  //Run the callback: When the port/IP has been assigned
#ifdef _WIN32
  typedef int socklen_t;
#endif

  //Retrieve socket information
  struct sockaddr_in sin;
  socklen_t len = sizeof(struct sockaddr_in);
  if(getsockname(this->_fd, (struct sockaddr*) &sin, (socklen_t*) &len) < 0)
    fprintf(stderr, "Unable to gather information from socket serving from!");
  else
    this->_port = ntohs(sin.sin_port);



  //Run port assignment callback, then the listener
  this->whenPortIsAssigned();
  this->listeningThread();

  //Return false; We should not exit out of this unless there is some awry interrupt
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

    //Increment number of clients currently connected
    this->_connectedClientsMutex.lock();
    this->_connectedClients++;
    this->_connectedClientsMutex.unlock();



    //Accept a new connection
    connection->_fd = accept(
      this->_fd,
      (struct sockaddr*) connection->_addrinf,
      (socklen_t*) &ss
    );



    //We failed to accept the connection for whatever reason
    if(connection->_fd < 0)
    {
      //Fatal errors (or maybe we used the stop function to invalidate the
      //socket
      if(errno == EBADF || errno == ECONNABORTED || errno == EINVAL)
      {
        delete connection;
        break;
      }

      //If it's an error that can probably fix itself
      if(errno == EAGAIN || errno == EFAULT || errno == EWOULDBLOCK || errno == EMFILE || errno == ENFILE || errno == ENOBUFS || errno == ENOMEM || errno == EOPNOTSUPP || errno == EPROTO || errno == EPERM)
      {
        delete connection;
        continue;
      }

      /*
       *
       *     ^
       *    / \
       *   / | \   CAUTION: Additional checks need to be implemented. If there is
       *  /  .  \  some sort of wierd error (in the kernel), this will make the server
       * /_______\ enter an infinite loop until (this->stop()) is called
       *
       */


      //Something's really messed up! QUIT!
      delete connection;
      break;
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


    //Keep a reference to this object in the connection object for direct access
    connection->server = this;


    //Launch the worker in another thread (through a safety proxy)
    std::thread(&cy::TCPServer::threadLauncher, this, connection).detach();
  }
}


//Callback called when the port is assigned to the server
//Function necessary to define because it is virtual
void cy::TCPServer::whenPortIsAssigned()
{
}


//Proxies the thread to the worker. It allows safety measures to prevent
//a single error from taking down the entire server!
void cy::TCPServer::threadLauncher(void* connection)
{
  //Encapsulate in try-catch block to capture any possible exceptions
  try
  {
    worker((cy::TCPConnection*) connection);
    delete (TCPConnection*) connection;
    connection = NULL;
  }
  catch(std::exception& e)
  {
    if(connection != NULL)
      delete (TCPConnection*) connection;
  }
}


//Handles the connection. Function meant to be overriden by the
//programmer
//	Currently has a test routine written
void cy::TCPServer::worker(TCPConnection* connection)
{
  connection->write((void*) "Hello World!", 13);
  char buffer[1024];
  while(true)
  {
    if(connection->read(buffer, 1024) <= 0)
      break;
    buffer[1023] = '\0';
    connection->write(buffer, strlen(buffer));
  }
}
