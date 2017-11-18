#include "timer.h"
#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
//#include "DebugLogger.h"
#include "thread.h"

Timer::Timer()
{
  Initialize();
}

Timer::Timer(double value)
{
  Initialize();
  Set(value);  
}

void Timer::Set(double value)
{
  unsigned int now;

  if (value == -1) {    
    timer.mode= VTMR_NEVER;
    GetMonotonic(&now);
    timer.start= now;
  } else {
    timer.mode= VTMR_ACTIVE;
    GetMonotonic(&now);
    timer.timeout= now+(int)((value<VTMR_MAXTIMEOUT?value:VTMR_MAXTIMEOUT)*TicsPerSec);
    timer.start= now;
  }
}

int Timer::Timeout(void)
{
  unsigned int now;
      
  if (timer.mode == VTMR_UNINITIALIZED) timer.mode= VTMR_DONE;
  if (timer.mode == VTMR_DONE) return (1);
  if (timer.mode == VTMR_NEVER) return (0);
  GetMonotonic(&now);
  if (now < timer.start) now+= VTMR_WRAPAROUND;
  if (now < timer.timeout) return (0);
  timer.mode= VTMR_DONE;
  return (1);
}

double Timer::Remain(void)
{
  unsigned int now;

  if (timer.mode == VTMR_UNINITIALIZED) timer.mode= VTMR_DONE;
  if (timer.mode == VTMR_DONE) return (0);
  if (timer.mode == VTMR_NEVER) return (-1);
  GetMonotonic(&now);
  if (now < timer.start) now+= VTMR_WRAPAROUND;
  if (now >= timer.timeout) {
    timer.mode= VTMR_DONE;
    return (0);
  }
  return (((double)timer.timeout-(double)now)/TicsPerSec);
}

void Timer::Pause(double value)
{
  struct vtmr timer;
  struct timeval slp;
  double TimeRemain;
  
  TimeRemain = Remain();
  
  if (value < 0.01) {
    slp.tv_sec= 0; slp.tv_usec= value*1000000;
    select(0, 0, 0, 0, &slp);
    return;
  }

  for (Set(value); !Timeout(); ) {
    slp.tv_sec= 0; slp.tv_usec= 10000;
    select(0, 0, 0, 0, &slp);
  }
  
  if (TimeRemain) Set(TimeRemain - value);
    
}

double Timer::Elapsed(void)
{
  unsigned int now;

  if (timer.mode == VTMR_UNINITIALIZED) return (-1);
  GetMonotonic(&now);
  return (((double)now-(double)timer.start)/TicsPerSec);
}

int Timer::Elapsed(struct _ReadableTimer *pReadableTimer)
{  
  
  unsigned long long Now;
  
  Now = (unsigned long long)((double)(Elapsed())*1000);  
  //DebugLoggerD("Current application runtime %.3f secs", (double)((double)Now/1000));
  ConvTime(&Now, &(pReadableTimer->Years), 31557600*1000);
  ConvTime(&Now, &(pReadableTimer->Months), 2592000*1000);
  ConvTime(&Now, &(pReadableTimer->Days), 86400*1000);
  ConvTime(&Now, &(pReadableTimer->Hours), 3600*1000);
  ConvTime(&Now, &(pReadableTimer->Minutes), 60*1000);
  ConvTime(&Now, &(pReadableTimer->Secs), 1000);    
  ConvTime(&Now, &(pReadableTimer->mSecs), 0);
  /*DebugLoggerD("Current application runtime is %d years, %d months, %d days, %d hours, %d minutes, %d secs, %d msecs", pUptime->Years,
                                                                                                                       pUptime->Months,
                                                                                                                       pUptime->Days,
                                                                                                                       pUptime->Hours,
                                                                                                                       pUptime->Minutes,
                                                                                                                       pUptime->Secs,
                                                                                                                       pUptime->mSecs);*/
  return (1);
}

unsigned int Timer::ElapsedMsec(void)
{
  struct _ReadableTimer ReadableTimer;
  Elapsed(&ReadableTimer);
  return (ReadableTimer.Secs*1000+ReadableTimer.mSecs);
}


int Timer::ConvTime(unsigned long long *pNow, int *pField, const unsigned long long Period)
{
  assert (pNow);
  assert (pField);
  
  *pField = 0;
  
  if (!Period) 
  {
    (*pField) += *pNow;    
  }
  else 
  {
    while (*pNow >= Period)
    {
      
      *pNow -= Period;    
      (*pField)++;
    }
  }
  return (1);
}


  
int Timer::Initialize(void)
{
  if (Initialized)
  {
    //printf("New object (%p) - thread etc is already initialized", this);
  }
  else
  {
    /* initialize components to use for timer-system */
    //printf("New object (%f) FIRST initialization", this);
    Monotonic= 0;
    Onetic= (int)((double)(1/(double)TIMER_FREQ)*1000000);
    TicsPerSec= TIMER_FREQ;
    memset(&AbsTimer, 0, sizeof(AbsTimer));
    memset(&timer, 0, sizeof(timer));
    timer.mode = VTMR_UNINITIALIZED;
    pthread_mutex_init(&_ThreadLock, 0);
    //CREATETHREAD(&_ThreadHandle, this->timer_main_process);  
    pthread_create(&_ThreadHandle, NULL, this->timer_main_process, this);

    Initialized = 1; 
  }
  
}


void Timer::TimerMainLoop(void)
{

  int dif;
  struct timeval prv, now= { 0, 0 };

  int sign;

  gettimeofday(&now, 0);
  prv= AbsTimer= now;

  for (;;) {
    /* fetch "now" & handle large "time-jumps" */
    prv= now;
    gettimeofday(&now, 0);
    dif= (now.tv_sec-prv.tv_sec)*100 + (now.tv_usec-prv.tv_usec)/10000;
    if (dif < 0 || dif > JMPTRESHOLD) AbsTimer= now;

    /* check if we have timeout (in millisec to avoid overflow) */
    sign= (AbsTimer.tv_sec-now.tv_sec)*1000 + (AbsTimer.tv_usec-now.tv_usec)/1000;
    if (sign <= 0) {
      /* modify timer with ONE tic */
      AbsTimer.tv_usec+= Onetic;
      while (AbsTimer.tv_usec >= 1000000) { AbsTimer.tv_usec-= 1000000; AbsTimer.tv_sec++; }
      pthread_mutex_lock(&_ThreadLock);
      if (++Monotonic >= VTMR_WRAPAROUND) Monotonic= 0;
      pthread_mutex_unlock(&_ThreadLock);
    }
    /* release CPU while waiting */
    usleep(1000);
  }
}

int Timer::GetMonotonic(unsigned int *pNow)
{
  pthread_mutex_lock(&_ThreadLock);
  *pNow= Monotonic;
  pthread_mutex_unlock(&_ThreadLock);
}

pthread_t Timer::_ThreadHandle;
pthread_mutex_t Timer::_ThreadLock;
unsigned int Timer::Monotonic;
unsigned int Timer::Onetic;
double Timer::TicsPerSec;
struct timeval Timer::AbsTimer;  
int Timer::Initialized;

