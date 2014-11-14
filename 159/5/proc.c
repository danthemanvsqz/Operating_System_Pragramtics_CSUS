// proc.c, 159
// processes are here

//#include "util.h" // Utilities
#include "types.h"
#include "proc.h"
#include "isr.h"	// Used for ShowStatus (Shouldn't be in this assignment)
#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"
#include "q_mgmt.h"
#include "irq7.h"


void IdleProc()
{
   int i;

   while(1)
   {
      cons_printf("0 ");
      for(i=0;i<1666000;i++) IO_DELAY();
   }
}

void Init() // handles key events, move the handling code out of Kernel()
{
	char key;
	char testmsg[] = "A_Team\n";
	int i, pid, child_pid;
	msg_t msg;
	
	child_pid = Spawn(PrintDriver);
	pid = GetPid();
	MyBZero((char *) &msg, sizeof(msg_t)); // Blank Message
	strncpy(msg.bytes, testmsg, sizeof(msg.bytes));
	cons_printf("%s",msg.bytes);
	
	
	while(1) // keys are 'p' 'c' 'b' 'q'
	{
		cons_printf("%i ", pid);
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
		
		if(cons_kbhit()) // if keyboard on the target PC has been pressed
		{
			key = cons_getchar(); // get the pressed key

			switch(key) // see if it's one of the following for service
			{
				case 'p':
					MsgSnd(child_pid, &msg);
					break;
				case 'b': breakpoint(); break; // this stops when run in GDB mode
				case 'q': exit(0);
			} // switch(key)
		} // if(cons_kbhit())
	}
}


