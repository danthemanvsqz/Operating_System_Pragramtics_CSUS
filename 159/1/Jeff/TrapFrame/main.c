// TrapFrame/main.c, 159
//
// The solution of phase 1

//some include statements...
//then declare global data structures...

#include "main.h"
#include "spede.h"
#include "isr.h"
#include "q_mgmt.h"
#include "externs.h"
#include "proc.h" // SimpleProc()
#include "entry.h"



struct i386_gate *idt_table;


void main()
{
	//Call InitData() to initialize kernel data structure like in SimulCode
	InitData();
	InitControl();
	
	/*
	Spawn the IdleProc (PID 0). Creation of a process includes the
	  initialization of its trapframe in the user process stack
	*/
	
	cur_pid = 0; // select IdleProc, the only available process to run
	Loader(pcbs[0].tf_p);
}


// prime IDT with an entry
void SetIDTEntry(int entry_num, func_ptr_t entry_addr)
{
	struct i386_gate *gateptr = &idt_table[entry_num];
	fill_gate(gateptr, (int)entry_addr, get_cs(), ACC_INTR_GATE, 0);
}


// InitControl()for setting up the timer interrupt
void InitControl()
{	
	
	idt_table = get_idt_base(); // get where IDT is in RAM
	cons_printf("IDT is at memory location %u.\n", idt_table);
	
	SetIDTEntry(32, TimerEntry); // prime IDT with an entry
	
	outportb(0x21, ~0x01); // 0x21 PIC mask resgister, ~0x01 mask value
	
	EI(); // exec CPU instruction "sti" to enable intr bit in CPU EFLAGS register
}


// InitData()
void InitData()
{
	int i;

	// queue initializations, both queues are empty first
	InitQ(&avail_q);
	InitQ(&ready_q);

	for(i=1; i<NUM_PROC; i++) // init pcbs[], skip 0 since it's Idle Proc
	{
		pcbs[i].state = AVAIL;
		EnQ(i, &avail_q);
	}

	cur_pid = -1;  // no process is running initially
}


// this is kernel's process scheduler, simple round robin
void Scheduler() 
{
   if(cur_pid > 0) return; // when cur_pid is not 0 (IdleProc) or -1 (none)

   if(cur_pid == 0) pcbs[0].state = READY; // skip when cur_pid is -1

   if(EmptyQ(&ready_q)) cur_pid = 0; // ready q empty, use IdleProc
   else cur_pid = DeQ(&ready_q);     // or get 1st process from ready_q

   pcbs[cur_pid].state = RUN;   // RUN state for newly selected process
}


void Kernel(tf_t *tf_p) // kernel directly enters here when interrupt occurs
{
	char key;
	
	pcbs[cur_pid].tf_p = tf_p;
	
	// tf_p->intr_id tells what intr made CPU get here, pushed in entry.S
	switch(tf_p->intr_id)
	{
		case TIMER_INTR:
		TimerISR(); // this must include dismissal of timer interrupt
	break;
	}

	// still handles other keyboard-generated simulated events
	if(cons_kbhit()) // check if a key was pressed (returns non zero)
	{
		//Use cons_getchar() to get the key pressed (char type)
		key = cons_getchar(); // get the pressed key
		
		switch(key)
		{
			// on the type of key pressed, perform the associated ISR
			// but _NO_ TimerISR here (which occurs by itself automatically)
			case 'n':
			if(EmptyQ(&avail_q))
				cons_printf("No more process!\n");
			else
				{
					cur_pid = DeQ(&avail_q);
					SpawnISR(cur_pid);
				}
			break;
			case 'k': KillISR(); break;
			case 's': ShowStatusISR(); break;
			case 'b': breakpoint(); break; // this stops when run in GDB mode
			case 'q': exit(0);
		} // switch(key)
	} // if(cons_kbhit())

	Scheduler();                // select a process to run
	Loader(pcbs[cur_pid].tf_p); // run the process selected
}
