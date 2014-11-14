// sys_calls.h

#ifndef _SYS_CALLS_H_
#define _SYS_CALLS_H_

#include "types.h"

int GetPid();
void Sleep(int);
int Spawn(func_ptr_t);
int SemInit();
void SemWait();
void SemPost();
void MsgSnd(int, msg_t*);
void MsgRcv(int, msg_t*);

#endif
