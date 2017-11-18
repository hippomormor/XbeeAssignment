/*
 * Socket.h
 *
 */

#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <stdlib.h>
#include <stdio.h>
#include "fcntl.h"
#include <memory.h>


#ifdef __cplusplus
class Syscall
{
public:
  Syscall();
  ~Syscall();
  int Exec(char *pCmd);

};
#endif

#ifdef __cplusplus
#define extern_C extern "C"
#else
#define extern_C
#endif


#endif /* SYSCALL_H_ */
