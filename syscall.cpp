#include "syscall.h"

Syscall::Syscall()
{
  
}

Syscall::~Syscall()
{
  ;
}

int Syscall::Exec(char *pCmd)
{
  return (system(pCmd));
}

