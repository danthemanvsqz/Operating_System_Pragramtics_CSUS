// replacements.c, 159

#include "isr.h"
#include "externs.h"
#include "spede.h"
#include "types.h"
#include "q_mgmt.h"
#include "proc.h"


// Clears the user stack
void MyBzero(void *sp, int size)
{
	int i;
	for(i=0; i<size; i++)
		 ((char *)(sp))[i] = 0;
}

void printTrap(tf_t *tp)
{
	cons_printf("Segment Registers\n");
	cons_printf("gs     : %u\n", tp->gs);
	//cons_printf("_notgs : %u\n", tp->_notgs);
	cons_printf("fs     : %u\n", tp->fs);
	//cons_printf("_notfs : %u\n", tp->_notfs);
	cons_printf("es     : %u\n", tp->es);
	//cons_printf("_notes : %u\n", tp->_notes);
	cons_printf("ds     : %u\n", tp->ds);
	//cons_printf("_notds : %u\n", tp->_notds);
	cons_printf("\nRegister State Frame\n");
	cons_printf("edi    : %u\n", tp->edi);
	cons_printf("esi    : %u\n", tp->esi);
	cons_printf("ebp    : %u\n", tp->ebp);
	cons_printf("esp    : %u\n", tp->esp);
	cons_printf("ebx    : %u\n", tp->ebx);
	cons_printf("edx    : %u\n", tp->edx);
	cons_printf("ecx    : %u\n", tp->ecx);
	cons_printf("eax    : %u\n", tp->eax);
	cons_printf("\nInterrupt Type\n");
	cons_printf("intr_id: %u\n", tp->intr_id);
	cons_printf("\nState Frame\n");
	cons_printf("eip    : %u\n", tp->eip);
	cons_printf("cs     : %u\n", tp->cs);
	cons_printf("eflags : %u\n", tp->eflags);
}


void TimerISR()
{
	if(cur_pid != 0) // If 0 skip this and clear Interrupt
	{
		pcbs[cur_pid].tick_count++;

		if(pcbs[cur_pid].tick_count == TIME_SLICE) // running up time, preempt it?
		{
			pcbs[cur_pid].tick_count = 0; // reset (roll over) usage time
			pcbs[cur_pid].total_tick_count += TIME_SLICE; // sum to total

			pcbs[cur_pid].state = READY;  // change its state
			EnQ(cur_pid, &ready_q);       // move it to ready_q

			cur_pid = -1;                 // no longer running
		}
	}
	
	outportb(0x20, 0x60); // 0x21 is PIC control reg, 0x60 dismisses IRQ 0
}


void SpawnISR(int pid)
{
	//Clear the user stack space for given pid.
	MyBzero((void *)user_stacks[pid], USER_STACK_SIZE );
	
	// 1st. point to just above of user stack, then drop by 64 bytes (tf_t)
	pcbs[pid].tf_p = (tf_t *)&user_stacks[pid][USER_STACK_SIZE];
	pcbs[pid].tf_p--;    // pointer arithmetic, now points to trapframe
	
	// fill in CPU's register context
	pcbs[pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
	pcbs[pid].tf_p->eip = (unsigned int)SimpleProc; // new process code
	pcbs[pid].tf_p->cs = get_cs();
	pcbs[pid].tf_p->ds = get_ds();
	pcbs[pid].tf_p->es = get_es();
	pcbs[pid].tf_p->fs = get_fs();
	pcbs[pid].tf_p->gs = get_gs();

	pcbs[pid].tick_count = pcbs[pid].total_tick_count = 0;
	pcbs[pid].state = READY;

	if(pid != 0) EnQ(pid, &ready_q);  // IdleProc (PID 0) is not queued
}

void KillISR()
{
   if(cur_pid < 1) return; // skip Idle Proc or when cur_pid has been set to -1
      
   pcbs[cur_pid].state = AVAIL;
   EnQ(cur_pid, &avail_q);
   cur_pid = -1;          // no proc running any more
}        

void ShowStatusISR()
{
   int i;
   int j = avail_q.head;
   char *state_names[] = {"AVAIL\0", "READY\0", "RUN\0", "SLEEP\0", "WAIT\0"};
	
	cons_printf("\n");
   for(i=0; i<NUM_PROC; i++)
   {
      cons_printf("pid %i, state %s, tick_count %i, total_tick_count %i \n",
             i, 
             state_names[pcbs[i].state],
             pcbs[i].tick_count,
             pcbs[i].total_tick_count);
   }
	//Added to show pid of each queue. (Jeff)
	
	cons_printf("Available Queue: ");
	for(i=0; i<avail_q.count; i++)
		cons_printf("%i ", avail_q.q[(j++)%Q_SIZE]);
	cons_printf("\n");

	j=ready_q.head;
	cons_printf("Ready Queue: ");
	for(i=0; i<ready_q.count; i++)
		cons_printf("%i ", ready_q.q[(j++)%Q_SIZE]);
	cons_printf("\n");

}
