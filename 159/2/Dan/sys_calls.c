// sys_calls.c

#include "sys_calls.h"

int GetPid()
{
	int pid;

	asm("int $48; movl %%eax, %0"
		: "=g" (pid)
		:
		: "eax");

	return pid;
}

void Sleep(int sleep_sec) // sleep for how many sec
{
	asm("movl %0, %%eax; int $49"	// copy sec to eax, call intr
		:                    	// no output
		: "g" (sleep_sec)    	// 1 input
		: "eax");               // 1 overwritten register
}
