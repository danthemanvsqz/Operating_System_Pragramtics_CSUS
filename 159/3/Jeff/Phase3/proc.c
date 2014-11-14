// proc.c, 159
// processes are here

#include "proc.h"
#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"

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
	int i, pid, child_pid;
	
	
	product_num = 0;
	common_sid = SemInit(1);
	pid = GetPid();

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
					break;
				case 'c':
					child_pid = Spawn(Consumer);
					break;
				case 'b': breakpoint(); break; // this stops when run in GDB mode
				case 'q': exit(0);
			} // switch(key)
		} // if(cons_kbhit())
	}
}


void Producer()
{
	int pid = GetPid();
	int sleep_sec = (pid % 5) + 1;
	int i;

	while(1)
	{
		SemWait(common_sid); // into critical section to access product
		
		cons_printf("Producer %i is producing...\n", pid);
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
		product_num++;
		cons_printf("Product count is now %i\n", product_num);

		SemPost(common_sid); // out of critical section

		Sleep(sleep_sec);
	}
}


void Consumer()
{
	int pid = GetPid();
	int sleep_sec = (pid % 5) + 1;
	int i;

	while(1)
	{
		Sleep(sleep_sec);

		SemWait(common_sid); // into critical section to access product

		if(product_num < 1) // access (read) product number first, if any
			cons_printf("Consumer %i wants to consume but no product.\n", pid);
		else
		{
			cons_printf("Consumer %i is consuming...\n", pid);
			product_num--;
			for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
			cons_printf("Product count is now %i\n", product_num);
		}

		SemPost(common_sid); // out of the critical section
	}
}
