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
	int i, pid, child_pid, sleep_time;
	msg_t msg;
	
	
	pid = GetPid();
	MyBZero((char *) &msg, sizeof(msg_t)); // Blank Message
	
	for(i=0;i<BUFF_SIZE;i++) // Fill Producer Message Queue.
		MsgSnd(0, &msg);
		
	
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
					child_pid = Spawn(Producer);
					sleep_time = (child_pid % 5) + 1;
					msg.numbers[0] = 0; // Recieve on 0.
					msg.numbers[1] = 1; // Send to 1.
					msg.numbers[2] = sleep_time; // Sleep for sleep_time seconds.
					for(i=0;i<NUM_BYTE;i++) msg.bytes[i] = 0; // Unsused string.
					MsgSnd(child_pid, &msg);
					break;
				case 'c':
					child_pid = Spawn(Consumer);
					sleep_time = (child_pid % 5) + 1;
					msg.numbers[0] = 1; // Recieve on 1.
					msg.numbers[1] = 0; // Send to 0.
					msg.numbers[2] = sleep_time; // Sleep for sleep_time seconds.
					for(i=0;i<NUM_BYTE;i++) msg.bytes[i] = 0; // Unsused string.
					MsgSnd(child_pid, &msg);
					break;
				case 's':
					ShowStatusISR();
					break;
				case 'b': breakpoint(); break; // this stops when run in GDB mode
				case 'q': exit(0);
			} // switch(key)
		} // if(cons_kbhit())
	}
}


void Producer()
{
	msg_t my_msg;     // this is local, repeatedly used in loop
	int i, my_pid, sleep_sec, get, send;
	char *fruits[] = {"apple","banana","cherry","durian","eggplant","fig","guava"};

	my_pid = GetPid();
	MsgRcv(my_pid, &my_msg);
	get = my_msg.numbers[0]; // Recieve on.
	send = my_msg.numbers[1]; // Send to.
	sleep_sec = my_msg.numbers[2]; // Sleep seconds.

	while(1)
	{
		MsgRcv(get, &my_msg); // get a msg (as product container)
		cons_printf("Producer %i is producing %s.\n", my_pid, fruits[my_pid % 7]);
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
		MyStrCpy(my_msg.bytes, fruits[my_pid % 7]);
		MsgSnd(send, &my_msg); // send it to be consumed
		
		Sleep(sleep_sec);
	} // forever loop
} // Producer()


void Consumer()
{
	msg_t my_msg;     // this is local, repeatedly used in loop
	int i, my_pid, sleep_sec, get, send;

	my_pid = GetPid();
	MsgRcv(my_pid, &my_msg);
	get = my_msg.numbers[0]; // Recieve on.
	send = my_msg.numbers[1]; // Send to.
	sleep_sec = my_msg.numbers[2]; // Sleep seconds.
	
	while(1)
	{
		Sleep(sleep_sec);
		MsgRcv(get, &my_msg);          // get a product
		cons_printf("Consumer %i is consuming %s...\n", my_pid, my_msg.bytes);
		for(i=0;i<1666000;i++) IO_DELAY();		// delay for about 1 sec
		MyStrCpy(my_msg.bytes, ""); 		// no product in it now
		MsgSnd(send, &my_msg);          // send it to producers
		
	} // forever loop
} // Consumer()


