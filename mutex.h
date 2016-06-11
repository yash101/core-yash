#ifndef MUTEX_H
#define MUTEX_H
#include "core_yash.h"

namespace cy
{
  const static int UNLOCKED = 0;
  const static int LOCKED = 1;

  class Mutex
  {
  private:
    void* _mutex;
    bool _allocated;
    bool _locked;
  public:
    Mutex();
    Mutex(int status);
    ~Mutex();
    void lock();
    void unlock();
  };

  //A simple way to use the stack to make sure we release mutexes!
  class AutomaticMutex
  {
  private:
    Mutex& _mtx;
  public:
    AutomaticMutex(Mutex& mtx) :
      _mtx(mtx)
    {
      _mtx.lock();
    }

    ~AutomaticMutex()
    {
      _mtx.unlock();
    }
  };
}

#endif // MUTEX_H
