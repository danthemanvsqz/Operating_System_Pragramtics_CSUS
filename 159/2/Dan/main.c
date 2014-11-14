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
#include "pq_mgmt.h"

// globals:
int cur_pid;                        // current running PID, -1 means no process
q_t ready_q, avail_q;               // processes ready to run and not used
pcb_t pcbs[NUM_PROC];               // process table
char user_stacks[NUM_PROC][USER_STACK_SIZE]; // run-time stacks for processes
pq_t sleep_q;
int sys_tick;

struct i386_gate *idt_table;


void main()
{
	//Call InitData() to initialize kernel data structure like in SimulCode
	InitData();
	
	cur_pid = 0; // select IdleProc, the only available process to run
	SpawnISR(0);
	
	InitControl();
	
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
	SetIDTEntry(GETPID_INTR, GetPidEntry);
	SetIDTEntry(SLEEP_INTR, SleepEntry);
	
	outportb(0x21, ~0x01); // 0x21 PIC mask resgister, ~0x01 mask value
	
	//EI(); // exec CPU instruction "sti" to enable intr bit in CPU EFLAGS register
}


// InitData()
void InitData()
{
	int i;
	
	sys_tick = 0;
	// queue initializations, both queues are empty first
	InitQ(&avail_q);
	InitQ(&ready_q);
	InitPQ(&sleep_q);

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
		case SLEEP_INTR:
			SleepISR();
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
