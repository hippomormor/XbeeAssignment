#include <stdlib.h>
#include <stdio.h>
#include "fcntl.h"
#include <memory.h>

#ifndef MESSAGEDEF_H_
#define MESSAGEDEF_H_

// messages (id)
#define GET_DATA_FROM_BACKEND       0
#define SET_DATA_FROM_HIGHEND       1
#define SET_DATA_FROM_BACKEND       2

// keys (actions)
#define ACTION_START              100
#define ACTION_STOP               101
#define ACTION_PAUSE              102
#define ACTION_KEY_F1             103
#define ACTION_KEY_F2             104
#define ACTION_KEY_F3             105
#define ACTION_KEY_F4             106
#define ACTION_KEY_UP             107
#define ACTION_KEY_DOWN           108
#define ACTION_KEY_ENTER          109
#define ACTION_KEY_ESC            110



// keys (other)
#define MOTOR_SET_RPM            1000
#define FIRMWARE_REV             1001
#define BSP_REV                  1002
#define COVER_STATE              1003
#define IP_ADDRESS_ETH0          1004
#define IP_ADDRESS_ETH1          1005
#define UPPER_AIR_OUT_VALVE      1006
#define UPPER_AIR_IN_VALVE       1007
#define AC_MOTOR_ACTUAL_SPEED    1008
#define AC_MOTOR_ACTUAL_CURRENT  1009
#define E_STOP_STATE             1010

struct _MessageDef
{
  int key;
  char name[256];
  int highend_can_set;
};

// message from application to debugprocess
struct _DbgMsg
{
  int Type;
  char Time[128];
  char FileName[256];
  int LineNum;
  char Msg[512];
};

// message from ewail to application
struct _ewa_msg
{
  int id;
  int key;
  char value[256];
};

#ifdef __cplusplus
class MessageDef
{
public:
  MessageDef();
  ~MessageDef();
  int Add(int Key, char *pName, int HighendCanSet);  
  char* GetName(int Key);
  int GetId(char *pName);
  int IsValid(char *pName);
  int IsValid(int Key);
  int CanHighendSet(int Key);

private:
  static struct _MessageDef *pMessageDef;
  static int Initialized;
  static int NumMessageDef;
  static int NumMessageDefAllocated;
};
#endif

#endif
