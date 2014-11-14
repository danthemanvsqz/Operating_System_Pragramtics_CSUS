// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"
#include "proc.h"

void IdleProc()
{
   int i;

   while(1)
   {
      cons_printf("0 ");
      for(i=0;i<1666000;i++)IO_DELAY();
   }
}

void Init() // handles key events, move the handling code out of Kernel()
{
   int pid, child_pid, i;
   char key;
   
   product_num = 0;
   //get "common_sid" thru SemInit() call
   common_sid = SemInit(1);

   //forever loop: // keys are 'p' 'c' 'b' 'q'
   for(;;)
   {
      //Displays its own PID (use GetPid call)
      pid = GetPid();
      //cons_printf("%d ", pid);
      //calls IO_DELAY for 1 second ( 
      for(i=0;i<1666000;i++) IO_DELAY();
      if (cons_kbhit())
      {
         //get the keyed-in char and
         key = cons_getchar();
         //service it: 'p' 'c' 'b' 'q' // 's' optional, can use GDB anyway
         switch(key) {
            case 'p':
               child_pid = Spawn(Producer);
               break;
            case 'c':
               child_pid = Spawn(Consumer);
               break;
            case 'b':
               breakpoint();
               break;
            case 'q':
               exit(0);
               break;         
         }
         //(eliminate 'n' and 'k' cases)
      }
   }
}

void Producer()
{
   //determine its sleep seconds (1-5) according to its PID like
   //in the previous project phase
   int pid = GetPid();
   int sleep_sec = pid % 5 + 1;
   int i;

   //forever loop:
   for(;;)
   {
      SemWait(common_sid); // into critical section to access product

      cons_printf("Producer %d is producing...\n", pid);
      //use a loop on IO_DELAY() to get 1 second to simulate production
      for(i=0;i<1666000;i++) IO_DELAY();
      product_num++;
      cons_printf("Product count is now %d\n", product_num);

      SemPost(common_sid); // out of critical section

      Sleep(sleep_sec);
   }
}

void Consumer()
{
   //determine the sleep seconds the same way as the producer
   int pid = GetPid();
   int sleep_sec = pid % 5 + 1;
   int i;

   //forever loop:
   for(;;)
   {
      //Sleep for sleep_sec
      Sleep(sleep_sec);

      SemWait(common_sid); // into critical section to access product

      if(product_num == 0)
      {
         cons_printf("Consumer %d wants to consume but no product...\n", pid);
      }
      else
      {
         cons_printf("Consumer %d is consuming...\n", pid);
         product_num--;
         for(i=0;i<1666000;i++) IO_DELAY();
         cons_printf("Product count is now %d\n", product_num);
      }

      SemPost(common_sid); // out of the critical section
   }
}

