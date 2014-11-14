// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"
#include "proc.h"
#include "q_mgmt.h"

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
   int i, key, cpid;//, sleep;
   msg_t primer;

   product_num = 0;
   common_sid = SemInit(1);
   
   for(i = 0; i < BUFF_SIZE; i++){
      MsgSnd(0, &primer);
   }

   cons_printf("Init loops: IO_DELAY 833000 times instead of 1666000...\n");

   for(;;)
   {
      //cons_printf("%d ", GetPid());
      for(i=0; i<833000; i++)IO_DELAY();

      if (cons_kbhit())
      {
         key = cons_getchar();
         switch(key)
         {
            case 'p':
               cpid = Spawn(Producer);
               primer.numbers[0] = 0;
               primer.numbers[1] = 1;
               primer.numbers[2] = cpid % 5 + 1;
               MsgSnd(cpid, &primer);
               break;
            case 'c':
               cpid = Spawn(Consumer);
               primer.numbers[0] = 0;
               primer.numbers[1] = 1;
               primer.numbers[2] = cpid % 5 + 1;
               break;
            case 'b':
               breakpoint();
               break;
            case 'q':
               exit(0);
         } // switch
      } // if
   } // for
} // Init()

void Producer()
{
   msg_t my_msg;     // this is local, repeatedly used in loop
   int i, my_pid, sleep_sec, producer_mid, consumer_mid;
   char *fruits[]=
      {"apple","banana","cherry","durian","eggplant","fig","guava"};
   

   my_pid = GetPid();
  
   
   MsgRcv(my_pid, &my_msg);
   consumer_mid = my_msg.numbers[0];
   producer_mid = my_msg.numbers[1];
   sleep_sec = my_msg.numbers[2];
   for(;;)
   {  
      MsgRcv(producer_mid, &my_msg); // get a msg (as product container)
      cons_printf("Producer %d produces a %s...\n", my_pid, my_msg.bytes);
      
      
      for(i=0; i<1666000; i++)IO_DELAY();
      MyStrCpy((char*) &my_msg.bytes, (char*) fruits[my_pid % 7]);
      MsgSnd(consumer_mid, &my_msg);
      Sleep(sleep_sec);
   }
}

void Consumer()
{  
   msg_t my_msg;     // this is local, repeatedly used in loop
   int i, my_pid, sleep_sec, producer_mid, consumer_mid;
   
   my_pid = GetPid();
   MsgRcv(my_pid, &my_msg);
   consumer_mid = my_msg.numbers[0];
   producer_mid = my_msg.numbers[1];
   sleep_sec = my_msg.numbers[2];
   
   for(;;){
  
      Sleep(sleep_sec);
      
      MsgRcv(consumer_mid, &my_msg);          // get a product
      
      cons_printf("** Consumer %d consumes a %s...\n", my_pid, my_msg.bytes);
      for(i=0; i<1666000; i++) IO_DELAY();
      MyStrCpy(my_msg.bytes, "Empty");
      MsgSnd(producer_mid, &my_msg);          // send it to producers
   }
}
