#ifndef SHARED_TIME_H_
#define SHARED_TIME_H_

#include <pthread.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>


struct _ReadableTimer
{
  int Years;
  int Months;
  int Days;
  int Hours;
  int Minutes;
  int Secs;
  int mSecs;
};

struct vtmr {
  int mode;                             // timer mode (0=inactive, 1=in_use)
  unsigned long start;                  // start value
  unsigned long timeout;                // timeout value
};


#define TIMER_FREQ    1000
#define VTMR_WRAPAROUND 0x80000000          // timer wrap-around value (3.74 year @ 18.2Hz timer)
#define VTMR_MAXTIMEOUT (3*365*24*60*60)    // Maximum timeout in seconds (18.2Hz timer)
#define JMPTRESHOLD 200                     // treshold (1/100s) to make it a "time jump"

enum {
  VTMR_UNINITIALIZED= 0,
  VTMR_ACTIVE,
  VTMR_DONE,
  VTMR_NEVER
};


#ifdef __cplusplus
class Timer
{
public:
  Timer();
  Timer(double value);
  void Set(double value);
  int Timeout(void);
  double Remain(void);
  void Pause(double value);
  double Elapsed(void);
  int Elapsed(struct _ReadableTimer *pReadableTimer);
  unsigned int ElapsedMsec(void);


private:
  /* prototypes */  
  int Initialize();
  int ConvTime(unsigned long long *pNow, int *pField, const unsigned long long Period);  
  int GetMonotonic(unsigned int *pNow);  
  
  /* local variables */
  struct vtmr timer;                       // main handle for the actual timer
  
  /* static variables */
  static int Initialized;
  static pthread_t _ThreadHandle;                 // handle to the main loop thread
  static pthread_mutex_t _ThreadLock;             // lock for this thread  
  static unsigned int Monotonic;
  static unsigned int Onetic;
  static double TicsPerSec;
  static struct timeval AbsTimer;  
  
  static void *timer_main_process(void *This)
  {
    ((Timer*)This)->TimerMainLoop();
  }
  void TimerMainLoop();
};


#endif
  
#ifdef __cplusplus
#define extern_C extern "C"
#else
#define extern_C
#endif

#endif


