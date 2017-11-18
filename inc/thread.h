#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <errno.h>

#ifdef __cplusplus
class Thread
{
public:
  Thread();
  
  static Thread* GetInstance(void){return my_instance;};
  int Create(pthread_t *pThreadHandle, void *(*pFunc)(void*));

private:
  static Thread* my_instance;
  
};
#endif


#ifdef __cplusplus
#define extern_C extern "C"
#else
#define extern_C
#endif

#define CREATETHREAD(a,b) Thread::GetInstance()->Create(a,b)
  
#endif


