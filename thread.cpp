#include "thread.h"
#include <assert.h>
#include <stdio.h>


Thread* Thread::my_instance = new Thread();
  
Thread::Thread()
{
  ;
}

int Thread::Create(pthread_t *pThreadHandle, void *(*pFunc)(void*))
{
  /* ensure parameters are correct */
  assert(pThreadHandle);
  assert(pFunc);
  
  /* create the thread */
  pthread_create(pThreadHandle, NULL, pFunc, NULL);

  return 0;
}


