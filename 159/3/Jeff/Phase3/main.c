// Team Name: A-Team
// Members: Dan vasquez, Jeff Roberts
// main.c, 159

#include "spede.h"
#include "types.h"
#include "main.h"
#include "q_mgmt.h"
#include "entry.h" // TIMER_INTR 32 & addr of TimerEntry
#include "isr.h"
#include "pq_mgmt.h"
#include "proc.h"

// globals:
struct i386_gate *idt_table;
int cur_pid, sys_tick, product_num, common_sid;								// current running PID, -1 means no process
q_t ready_q, avail_q, avail_sem_q;               	// processes ready to run and not used
pq_t sleep_q;
pcb_t pcbs[NUM_PROC];								// process table
char user_stacks[NUM_PROC][USER_STACK_SIZE];		// run-time stacks for processes
sem_t sems[NUM_SEM];



void main() // Bootsraping and starting idle process.
{
   InitData();  // initialize needed data for kernel
   InitControl();
   SpawnISR(0, IdleProc); // create IdleProc for OS to run if no user processes
   SpawnISR(1, Init);
   cur_pid = 1;
   Loader(pcbs[1].tf_p);
}

void SetIDTEntry(int entry_num, func_ptr_t entry_addr)
{
   struct i386_gate *gateptr = &idt_table[entry_num];
   fill_gate(gateptr, (int)entry_addr, get_cs(), ACC_INTR_GATE,0);
}


void InitControl()
{
	idt_table = get_idt_base(); // get where IDT is in RAM
	SetIDTEntry(32, TimerEntry); // prime IDT with an entry

	SetIDTEntry(GETPID_INTR, GetPidEntry);
	SetIDTEntry(SLEEP_INTR, SleepEntry);
	SetIDTEntry(SPAWN_INTR, SpawnEntry);
	SetIDTEntry(SEMINIT_INTR, SemInitEntry);
	SetIDTEntry(SEMWAIT_INTR, SemWaitEntry);
	SetIDTEntry(SEMPOST_INTR, SemPostEntry);

	outportb(0x21, ~1);
}


void InitData()
{
	int i;

	InitQ(&avail_q); // set queues initially empty
	InitQ(&ready_q);
	InitPQ(&sleep_q);
	InitQ(&avail_sem_q);

	sys_tick = 0;

	for(i=2; i<NUM_PROC; i++) // init pcbs[], skip 0 since it's Idle Proc
	{
		pcbs[i].state = AVAIL;
		EnQ(i, &avail_q);
	}

	for(i=NUM_SEM-1; i>=0; i--)
		EnQ(i, &avail_sem_q);
	
	cur_pid = -1;  // no process is running initially
}


void Scheduler() // this is kernel's process scheduler, simple round robin
{
   if(cur_pid > 0) return; // when cur_pid is not 0 (IdleProc) or -1 (none)

   if(cur_pid == 0) pcbs[0].state = READY; // skip when cur_pid is -1

   if(EmptyQ(&ready_q)) cur_pid = 0; // ready q empty, use IdleProc
   else cur_pid = DeQ(&ready_q);     // or get 1st process from ready_q

   pcbs[cur_pid].state = RUN;   // RUN state for newly selected process
}


void Kernel(tf_t *tf_p) // kernel begins its control upon/after interrupt
{
	int pid, sid;
	pcbs[cur_pid].tf_p = tf_p;
	
	switch(tf_p->intr_id)
	{
		case TIMER_INTR:
			TimerISR(); // service the simulated timer interrupt
			break;
		case GETPID_INTR:
			tf_p->eax = cur_pid;
			break;
		case SLEEP_INTR:
			SleepISR(); 
			break;
		case SPAWN_INTR:
			if(EmptyQ(&avail_q))
			{
				cons_printf("No more processes!\n");
				tf_p->eax = -1;	
			}
			else
			{
				pid = DeQ(&avail_q);
				if(pid == 0)
					SpawnISR(pid, IdleProc);
				else
				{
					pcbs[pid].state = READY;
					SpawnISR(pid, (func_ptr_t)(tf_p->eax));
				}
				tf_p->eax = pid;
			}
			break;
		case SEMINIT_INTR:
			sid = SemInitISR(tf_p->eax);
			tf_p->eax = sid;
			break;
		case SEMWAIT_INTR:
			SemWaitISR(common_sid);
			break;
		case SEMPOST_INTR:
			SemPostISR(common_sid);
			break;
	}

   Scheduler();                // find same/new process to run
   Loader(pcbs[cur_pid].tf_p); // load it to run
} // Kernel()

