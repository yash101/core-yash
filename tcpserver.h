#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "core_yash.h"
#include "mutex.h"
#include <stddef.h>
#include <stdint.h>
#include <vector>

namespace cy
{
  class TCPConnection
  {
    friend class TCPServer;
  private:
    int _fd;
    void* _addrinf;
    void* _tv;
  public:
    TCPServer* server;
    TCPConnection();
    ~TCPConnection();

    int write(void* data, size_t len);
    int read(void* arr, size_t len);
  };

  class TCPServer
  {
    friend class TCPConnection;
  private:

    int _fd;
    void* _addrinf;
    void* _timeoutStruct;
    int16_t _port;

    //Mutex for isRunning, timeoutUsec, timeoutSec and connectionQueueSize
    //****IMPLEMENT****//

    bool _isRunning;
    bool _stop;

    int _connectionQueueSize;

    int _connectedClients;
    int _maxConnectedClients;

    cy::Mutex _connectedClientsMutex;
    cy::Mutex _maxConnectedClientsMutex;

    inline void initVars()
    {
      _fd = 0;
      _port = 0;
      _connectionQueueSize = 3;
      _connectedClients = 0;
      _maxConnectedClients = 0;

      _isRunning = false;
      _stop = false;

      _addrinf = NULL;
      _timeoutStruct = NULL;
    }

  protected:

    void listeningThread();
    void threadLauncher(void* connection);

    virtual void whenPortIsAssigned();
    virtual void worker(TCPConnection* connection);

  public:

    TCPServer();
    ~TCPServer();

    bool setMaxConnectedClients(int max);
    int getMaxConnectedClients();
    int getConnectedClientsCount();

    bool setPort(int16_t portno);
    int16_t getPort();

    bool setConnectionQueueSize(int qs);
    int getConnectionQueueSize();

    bool setTimeoutUsec(int usec);
    bool setTimeoutSec(int sec);
    bool setTimeout(int usec, int sec);
    int getTimeoutUsec();
    int getTimeoutSec();

    bool isServerRunning();
    bool stopServer();


    bool startServer();

  };
}

#endif // TCPSERVER_H
