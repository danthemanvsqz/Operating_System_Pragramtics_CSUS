// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"
#include "proc.h"
#include "q_mgmt.h"     // MyStrCpy()
#include "irq7.h"

void IdleProc() {
   int i;

   while(1) {
      cons_printf("0 ");
      for(i=0;i<1666000;i++)IO_DELAY();
   }
}

void Init() { // handles key events, move the handling code out of Kernel()
   int i, key, cpid, pid;
   msg_t my_msg;
	char testmsg[] = "A_Team\n";

   cpid = Spawn(PrintDriver);
   pid = GetPid();
   MyBZero((char*) &my_msg, sizeof(msg_t));
   MyStrCpy(my_msg.bytes, testmsg);
   cons_printf("%s", my_msg.bytes);

cons_printf("**********************************************************\n");
cons_printf("*     INIT'S PID PRINT & IO_DELAY LOOP COMMENTED OUT     *\n");
cons_printf("*         (in order to view the runtime better)          *\n");
cons_printf("**********************************************************\n");
   for(;;) {

      cons_printf("%d ", GetPid());
      for(i=0; i<833000; i++)IO_DELAY();

      if (cons_kbhit()) {
         key = cons_getchar();
         switch(key) {
            case 'p':
               MsgSnd(cpid, &my_msg);
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


