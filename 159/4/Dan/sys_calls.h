// sys_calls.h

#ifndef _SYS_CALLS_H_
#define _SYS_CALLS_H_
#include "types.h"

int GetPid();
void Sleep(int);
int Spawn(func_ptr_t addr);
int SemInit(int sem_count);
void SemWait(int sid);
void SemPost(int sid);

#endif
