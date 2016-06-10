#ifndef CORE_YASH_H
#define CORE_YASH_H

#if defined(__linux) || defined(__linux__) || defined(__unix__)
#define POSIX
#define PLATFORM "POSIX"
#else
#define NOT_POSIX
#define PLATFORM "NOT POSIX"
#endif

#define NOT_ALLOCATED 0
#define ALLOCATED 1

namespace cy
{
  class TCPServer;
  class TCPConnection;
  class TCPClient;
  class UDPServer;
  class UDPClient;
  class Mutex;
}

#endif // CORE_YASH_H
