// isr.c, 159
// ISRs called from Kernel()

#include "spede.h"
#include "types.h"
#include "isr.h"
#include "q_mgmt.h"
#include "externs.h"
#include "proc.h"

void SpawnISR(int pid)
{
   if(pid != 0) EnQ(pid, &ready_q); // avoid queuing IdleProc
   pcbs[pid].state = READY;
   pcbs[pid].tick_count = pcbs[pid].total_tick_count = 0;
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

void TimerISR()
{
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
