// proc.c, 159
// processes are here

#include "util.h" // Utilities
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
	int i, pid, mid;
	msg_t msg;
	
	mid = Spawn(PrintDriver);
	cons_printf("Mid %d\n", mid);
	pid = GetPid();
	MyBZero((char *) &msg, sizeof(msg_t)); // Blank Message
	/*
	for(i=0;i<BUFF_SIZE;i++) // Fill Producer Message Queue.
		MsgSnd(0, &msg);
		*/
	
	
	while(1) // keys are 'p' 'c' 'b' 'q'
	{
		cons_printf("%i ", pid);
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
		
		if(cons_kbhit()) // if keyboard on the target PC has been pressed
		{
			key = cons_getchar(); // get the pressed key
         //cons_printf("Hit\n");
			switch(key) // see if it's one of the following for service
			{
				case 'p':
				   MyStrCpy(msg.bytes, "Hello from the A-team\n");
				   MsgSnd(mid, &msg);
					PrintDriver();
					break;
				
				case 'b': breakpoint(); break; // this stops when run in GDB mode
				case 'q': exit(0);
			} // switch(key)
		} // if(cons_kbhit())
	}
}



