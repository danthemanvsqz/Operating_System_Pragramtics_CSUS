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

void Sleep(int sleep_secs) // sleep for how many secs
{
   asm("movl %0, %%eax; int $49"  // copy secs to eax, call intr
       :                            // no output
       : "g" (sleep_secs)           // 1 input
       : "eax");                    // 1 overwritten register
}

int Spawn(func_ptr_t addr) {
   int child_pid;
   
   asm("movl %1, %%eax; int $50; movl %0, %%eax"
      : "=g" (child_pid)
      : "g" ((int) addr)
      : "eax");
   
   return child_pid;
}

int SemInit(int sem_count) {
   int sid;
   
   if(sem_count < 0) return -1;
   
   asm("movl %1, %%eax; int $51; movl %%eax, %0"
      : "=g" (sid)
      : "g" (sem_count)
      : "eax");
   
   return sid;
}
void SemWait(int sid) {
   asm("movl %0, %%eax; int $52"
      :
      : "g" (sid)
      : "eax");

}

void SemPost(int sid) {
   asm("movl %0, %%eax; int $53"
      :
      : "g" (sid)
      : "eax");

}

