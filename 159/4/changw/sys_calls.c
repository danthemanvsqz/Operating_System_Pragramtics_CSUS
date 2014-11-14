// sys_calls.c

#include "types.h" // func_ptr_t

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
   asm("movl %0, %%eax; int $49"  // copy sec to eax, call intr
       :                          // no output
       : "g" (sleep_sec)          // 1 input
       : "eax");                  // 1 overwritten register
}

int Spawn( func_ptr_t addr )
{
   int pid;

   asm( "movl %1, %%eax; int $50; movl %%ebx, %0"
      : "=g" ( pid )
      : "g" ( (int) addr )
      : "eax", "ebx"
   );
   return pid;
}

int SemInit( int sem_count )
{
   int sid;

   asm( "movl %1, %%eax; int $51; movl %%ebx, %0"
      : "=g" ( sid )
      : "g" ( sem_count )
      : "eax", "ebx"
   );
   return sid;
}

void SemWait( int sid )
{
   asm( "movl %0, %%eax; int $52"
      :
      : "g" ( sid )
      : "eax"
   );
}

void SemPost( int sid )
{
   asm( "movl %0, %%eax; int $53"
      :
      : "g" ( sid )
      : "eax"
   );
}

void MsgSnd(int mid, msg_t* addr){
   asm( "movl %0, %%eax; movl %1, %%ebx; int $54"
      :
      : "g" (mid), "g" ((int) addr)
      : "eax", "ebx"
   );
}

void MsgRcv(int mid, msg_t* addr){
   asm( "movl %0, %%eax; movl %1, %%ebx; int $55"
      :
      : "g" (mid), "g" ((int) addr)
      : "eax", "ebx"
   );

}

