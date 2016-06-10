#include "mutex.h"
#ifdef POSIX
#include "pthread.h"
typedef pthread_mutex_t mutex_t;
#else
#include <mutex>
typedef std::mutex mutex_t;
#endif

cy::Mutex::Mutex() :
  _locked(false)
{
  _allocated = ((_mutex = new mutex_t) == NULL) ? false : true;
#ifdef POSIX
  if(_allocated)
  {
    (*((mutex_t*)_mutex)) = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init((mutex_t*) _mutex, NULL);
  }
#endif
}

cy::Mutex::Mutex(int status) :
  _locked(false)
{
  _allocated = ((_mutex = new mutex_t) == NULL) ? false : true;
#ifdef POSIX
  if(_allocated)
  {
    (*((mutex_t*)_mutex)) = PTHREAD_MUTEX_INITIALIZER;
    if(status == cy::LOCKED)
    {
      pthread_mutex_lock((mutex_t*) _mutex);
      _locked = true;
    }
  }
#endif
}

void cy::Mutex::lock()
{
  if(!_allocated) return;
#ifdef POSIX
  pthread_mutex_lock((mutex_t*) _mutex);
  _locked = true;
#else
  _mutex.lock();
#endif
}

void cy::Mutex::unlock()
{
  if(!_allocated) return;
#ifdef POSIX
  pthread_mutex_unlock((mutex_t*) _mutex);
  _locked = false;
#else
  _mutex.unlock();
#endif
}

cy::Mutex::~Mutex()
{
  if(!_allocated) return;
#ifdef POSIX
  if(_locked)
  {
    pthread_mutex_unlock((mutex_t*) _mutex);
  }
  pthread_mutex_destroy((mutex_t*) _mutex);
#else
  if(_locked)
    _mutex.unlock();
#endif

  delete ((mutex_t*) _mutex);
}
