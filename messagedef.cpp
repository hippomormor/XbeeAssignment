#include "messagedef.h"

MessageDef::MessageDef()
{
  if (!Initialized)
  {
    NumMessageDefAllocated = 0;
    NumMessageDef = 0;  
    pMessageDef = 0;

    #define ADDMSG(a,b) Add(a,#a,b)
    
    //ADDMSG(GET_DATA_FROM_BACKEND, 0);
    //ADDMSG(SET_DATA_FROM_HIGHEND, 0);
    //ADDMSG(SET_DATA_FROM_BACKEND, 0);
    
    ADDMSG(ACTION_START,1);
    ADDMSG(ACTION_STOP,1);
    ADDMSG(ACTION_PAUSE,1);  
    ADDMSG(ACTION_KEY_F1,1);
    ADDMSG(ACTION_KEY_F2,1);
    ADDMSG(ACTION_KEY_F3,1);
    ADDMSG(ACTION_KEY_F4,1);
    ADDMSG(ACTION_KEY_UP,1);
    ADDMSG(ACTION_KEY_DOWN,1);
    ADDMSG(ACTION_KEY_ENTER,1);
    ADDMSG(ACTION_KEY_ESC,1);

    ADDMSG(MOTOR_SET_RPM, 1);
    ADDMSG(FIRMWARE_REV, 0);
    ADDMSG(BSP_REV, 0);
    ADDMSG(COVER_STATE, 0);
    ADDMSG(IP_ADDRESS_ETH0,0);
    ADDMSG(IP_ADDRESS_ETH1,0);
    ADDMSG(UPPER_AIR_OUT_VALVE,0);
    ADDMSG(UPPER_AIR_IN_VALVE,0);
    ADDMSG(AC_MOTOR_ACTUAL_SPEED,0);
    ADDMSG(AC_MOTOR_ACTUAL_CURRENT,0);
    ADDMSG(E_STOP_STATE,0);
    
    Initialized = 1;
  }
}

char* MessageDef::GetName(int Key)
{
  int i;
  char* retval = 0;
  
  for (i=0;i<NumMessageDef;i++) {
    if (pMessageDef[i].key == Key) {
      // found
      retval = pMessageDef[i].name;      
      break;
    }
  }
  return (retval);
}

int MessageDef::GetId(char *pName)
{
  int i;
  int retval = 0;
  
  for (i=0;i<NumMessageDef;i++) {
    if (!strcmp(pMessageDef[i].name, pName)) {
      // found
      retval = pMessageDef[i].key;
      break;
    }
  }
  return (retval);
  
}
int MessageDef::IsValid(char *pName)
{
  return (GetId(pName) ? 1 : 0);
}

int MessageDef::IsValid(int Key)
{
  return (GetName(Key) ? 1 : 0);
}

int MessageDef::CanHighendSet(int Key)
{
  int i;
  int retval = 0;
  
  for (i=0;i<NumMessageDef;i++) {
    if (pMessageDef[i].key == Key) {
      // found
      retval = pMessageDef[i].highend_can_set;
      break;
    }
  }
  return (retval);  
}

int MessageDef::Add(int Key, char *pName, int HighendCanSet)
{
  struct _MessageDef *pTmp;
  
  if (!pMessageDef) {
    pMessageDef = (struct _MessageDef*)malloc(10*sizeof(struct _MessageDef));    
    NumMessageDefAllocated = 10;
  }
  
  if (NumMessageDef >= NumMessageDefAllocated) {
    // reallocate room for more pointers
    int NewSize = (NumMessageDefAllocated+10)*sizeof(struct _MessageDef);
    pTmp = (struct _MessageDef*)realloc(pMessageDef, NewSize);
    if (pTmp) {
      // success
      NumMessageDefAllocated+=10;
      pMessageDef = pTmp;
    } else {
      printf("\n\rERROR: Can't reallocate %d bytes", NewSize);
      fflush(stdout);
      return (0);
    }
  }
  
  // add data to new record
  pMessageDef[NumMessageDef].key = Key;  
  strcpy(pMessageDef[NumMessageDef].name, pName);
  pMessageDef[NumMessageDef].highend_can_set = HighendCanSet;
  NumMessageDef++;
  //printf("\n\rAdded record %d %s (now %d)", Key, pName, NumMessageDef);
  //fflush(stdout);
  return (1);  
}

int MessageDef::Initialized = 0;
struct _MessageDef* MessageDef::pMessageDef;
int MessageDef::NumMessageDef;
int MessageDef::NumMessageDefAllocated;


