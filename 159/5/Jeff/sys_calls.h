#ifndef _SYS_CALLS_H_
#define _SYS_CALLS_H_

#include "types.h"

int GetPid();
void Sleep(int);
int Spawn(func_ptr_t);
int SemInit(int);
void SemWait(int);
void SemPost(int);
void MsgSnd(int, msg_t *); // args: mid and addr of a local msg_t
void MsgRcv(msg_t *); // args: mid and addr of a local msg_t

#endif
