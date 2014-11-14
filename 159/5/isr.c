// isr.c, 159 1
// ISR's called from Kernel()

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "q_mgmt.h"
#include "pq_mgmt.h"
#include "isr.h"
//#include "util.h"


void ShowStatusISR()
{
   int i;
   char *names[]={"AVAIL\0","READY\0","RUN\0","SLEEP\0","WAIT\0"};

   for(i=0; i< NUM_PROC; i++)
   {
      cons_printf("pid %d, state %s, tick_count %d, total_tick_count %d\n",
             i,
             names[pcbs[i].state],
             pcbs[i].tick_count,
             pcbs[i].total_tick_count);
   }

   cons_printf("avail_q: ");
   ShowQ(&avail_q);

   cons_printf("ready_q: ");
   ShowQ(&ready_q);
}

void ShowQ(q_t *p)
{
   int i;

   for(i = 0; i < p->count; i++)
      cons_printf( "%d ", p->q[ ( p->head + i) % Q_SIZE ] ) ;

   cons_printf("\n"); 
}


int SemInitISR(int sem_count)
{
	int sid;
	sid = DeQ(&avail_sem_q);
	if(sid == -1) return sid;
	sems[sid].count = sem_count;
	InitQ(&(sems[sid].wait));
	
	return sid;
}


void SemWaitISR(int sid)
{
	if( sems[sid].count > 0)
	{
		sems[sid].count--;
		return;
	}
	
	EnQ(cur_pid, &(sems[sid].wait));
	pcbs[cur_pid].state = WAIT;
	cur_pid = -1;
	return;
}


void SemPostISR(int sid)
{
	int pid;
	
	if( EmptyQ( &(sems[sid].wait)) )
	{
		sems[sid].count++;
		return;
	}
	
	pid = DeQ(&(sems[sid].wait));
	pcbs[pid].state = READY;
	EnQ(pid, &ready_q);
	return;
}


void MsgSndISR(int mid, msg_t *p)
{
	int pid;
	
	p->sender = cur_pid;
	p->send_tick = sys_tick;
	
	if(EmptyQ(&(mboxes[mid].wait_q))) // If wait Queue of message is empty.
		MsgEnQ(p, &(mboxes[mid].msg_q)); // Queue the message.
	else
	{
		pid = DeQ(&(mboxes[mid].wait_q)); // Dequeue the waiting process
		pcbs[pid].state = READY;
		EnQ(pid, &ready_q);
		*((msg_t *) pcbs[pid].tf_p->eax) = *p;
	}
}


void MsgRcvISR(msg_t *p)
{	
	if(!MsgEmptyQ(&(mboxes[cur_pid].msg_q)))
		MsgDeQ(p, &(mboxes[cur_pid].msg_q));
	else
	{
		EnQ(cur_pid, &(mboxes[cur_pid].wait_q));
		pcbs[cur_pid].state = WAIT;
		cur_pid = -1;
	}
}


void SpawnISR(int pid, func_ptr_t addr)
{
	
	MyBZero(user_stacks[pid], USER_STACK_SIZE);
	MyBZero((char *) &mboxes[pid], sizeof(mbox_t));

	// 1st. point to just above of user stack, then drop by 64 bytes (tf_t)
	pcbs[pid].tf_p = (tf_t *)&user_stacks[pid][USER_STACK_SIZE];
	pcbs[pid].tf_p--;    // pointer arithmetic, now points to trapframe

	// fill in CPU's register context
	pcbs[pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;


	//pcbs[pid].tf_p->eip = (unsigned int)SimpleProc; // new process code
	pcbs[pid].tf_p->eip = (unsigned int)addr;

	pcbs[pid].tf_p->cs = get_cs();
	pcbs[pid].tf_p->ds = get_ds();
	pcbs[pid].tf_p->es = get_es();
	pcbs[pid].tf_p->fs = get_fs();
	pcbs[pid].tf_p->gs = get_gs();

	pcbs[pid].tick_count = pcbs[pid].total_tick_count = 0;
	pcbs[pid].state = READY;

	if(pid != 0) EnQ(pid, &ready_q);  // IdleProc (PID 0) is not queued
}
      

void TimerISR()
{
   int sleep_pid;
// dismiss IRQ 0 at PIC control reg (0x20), if IRQ 7, send 0x67)
   outportb(0x20, 0x60);

   sys_tick++;
   
   while( !EmptyPQ(&sleep_q) && ( peek(&sleep_q) <= sys_tick )){
      sleep_pid = DePQ(&sleep_q);
      EnQ(sleep_pid, &ready_q);
      pcbs[sleep_pid].state = READY;
   }

   if(cur_pid == 0) return; // if Idle process, no need to do this on it

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


void SleepISR()
{
   int sleep_sec = pcbs[cur_pid].tf_p->eax;
   //cons_printf("\nSleeping %i\n", cur_pid);
	pcbs[cur_pid].state = SLEEP;
	pcbs[cur_pid].wake_tick = sys_tick + sleep_sec * 100;
	EnPQ(cur_pid, pcbs[cur_pid].wake_tick, &sleep_q);
	cur_pid = -1;
}

