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


int Spawn(func_ptr_t addr){
	int child_pid;
	
	asm("movl %1, %%eax; int $50; movl %%eax, %0"
		: "=g" (child_pid)
		: "g" ((int)addr)
		: "eax");
		
		return child_pid;
}


int SemInit(int sem_count){
	int sid;
	
	if(sem_count<0) return -1;
	
	asm("movl %1, %%eax; int $51; movl %%eax, %0"
		: "=g" (sid)
		: "g" (sem_count)
		: "eax");
		
	return sid;
}


void SemWait(int sid)
{
	asm("movl %0, %%eax; int $52"	// Copy sid to eax, call intr
		:
		: "g" (sid)
		: "eax");
}


void SemPost(int sid)
{
	asm("movl %0, %%eax; int $53"	// Copy sid to eax, call intr
		:
		: "g" (sid)
		: "eax");
}


void MsgSnd(int mid, msg_t *msg_p) // args: mid and addr of a local msg_t
{
	asm("movl %0, %%eax; movl %1, %%ebx; int $54"
		:
		: "g" (mid), "b" ((int)msg_p)
		: "eax", "ebx");
}


void MsgRcv(msg_t *msg_p) // args: mid and addr of a local msg_t
{
	asm("movl %0, %%eax; int $55"
		: //"=g" ((msg_t *)msg_p)
		: "g" ((int)msg_p)
		: "eax"
	);
}


