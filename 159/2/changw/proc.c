// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"

void IdleProc()
{
   int i;

   while(1)
   {
      cons_printf("0 ");
      for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
   }
}

void SimpleProc()
{
   int pid, sleep_sec;
   
   while(1) {
      pid = GetPid();
      cons_printf("%i ", pid);
      sleep_sec = (pid % 5) + 1;
      Sleep(sleep_sec);
   }
}
