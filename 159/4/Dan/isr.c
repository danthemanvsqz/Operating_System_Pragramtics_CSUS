// isr.c, 159 1
// ISR's called from Kernel()

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "q_mgmt.h"
#include "isr.h"
#include "proc.h"

void MyBZero(char *p, int size)
{
   while( size-- ) *p++ = 0; // they'll be all nulls
}

// code your own MemCpy is not hard
// Do NOT use memcpy() in C Run-Time Lib (CRTL)
void MyMemCpy(char *dest, char *source, int size)
{
   while(size--) *dest++ = *source++;
}

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

void SpawnISR(int pid, func_ptr_t p)
{
   MyBZero(user_stacks[pid], USER_STACK_SIZE);

// 1st. point to just above of user stack, then drop by 64 bytes (tf_t)
   pcbs[pid].tf_p = (tf_t *)&user_stacks[pid][USER_STACK_SIZE];
   pcbs[pid].tf_p--;    // pointer arithmetic, now points to trapframe

// fill in CPU's register context
   pcbs[pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
   pcbs[pid].tf_p->eip = (unsigned int)p; // new process code
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
// dismiss IRQ 0 at PIC control reg (0x20), if IRQ 7, send 0x67)
   outportb(0x20, 0x60);

   WakyWaky();

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

void WakyWaky() // wake if (multiple) processes to wake
{
   int pid;

   sys_tick++; // system time/ticks

   // repeat until false:
   //    check if sleep_q is not empty and wake_tick of the 1st
   //    process indicated in sleep_q == sys_tick to wake it up
   while(!EmptyQ(&sleep_q) &&
         pcbs[sleep_q.q[sleep_q.head]].wake_tick==sys_tick)
   {
      pid = DeQ(&sleep_q);
      EnQ(pid, &ready_q);
      pcbs[pid].state = READY;
   }
}

void SleepISR()
{
   q_t tmp_q;

   InitQ( &tmp_q ); // empty temp q

// calc the future wake_tick of cur_pid
   pcbs[cur_pid].wake_tick = sys_tick + 100 * pcbs[cur_pid].tf_p->eax;

// WHILE sleep_q not empty AND the 1st one in it has a wake_tick <=
// wake_tick of cur_pid: move that 1st one from sleep_q to tmp_q
   while(!EmptyQ(&sleep_q)
         &&
         pcbs[ sleep_q.q[ sleep_q.head ] ].wake_tick
         <= pcbs[ cur_pid ].wake_tick
        )
   {
      EnQ(DeQ(&sleep_q), &tmp_q); // append it to tmp_q
   }

// insertion point is reached as either sleep_q is now empty or the
// wake_tick of cur_pid is smaller than the 1st one in the sleep_q
   EnQ( cur_pid, &tmp_q ); // append cur_pid to tmp_q

// append the rest of sleep_q to tmp_q
   while( ! EmptyQ( &sleep_q ) )
   {
      EnQ( DeQ( &sleep_q ), &tmp_q );
   }

// update sleep_q with the now ordered tmp_q, fastest:
   MyMemCpy((char *)&sleep_q, (char *)&tmp_q, sizeof(q_t));

   pcbs[cur_pid].state = SLEEP;
   cur_pid = -1; // need to get a different proc to run now
}


int SemInitISR(int sem_count) {
   int sid;
   sid = DeQ(&avail_sem_q);
   
   if(sid < 0) {
      return -1;
   }
   sems[sid].sem_count = sem_count;
   InitQ(&sems[sid].wait_q);
   return sid;
}

//void SemWaitISR(int sid) {
void SemWaitISR() {
   int sid = pcbs[cur_pid].tf_p->eax;
   if(sems[sid].sem_count > 0) {
      sems[sid].sem_count--;
   }
   else {
      EnQ(cur_pid, &sems[sid].wait_q);
      pcbs[cur_pid].state = WAIT;
      cur_pid = -1;
   }
}

//void SemPostISR(int sid) {
void SemPostISR() {
   int pid, sid = pcbs[cur_pid].tf_p->eax;
   
   if(!EmptyQ(&sems[sid].wait_q)) {
      pid = DeQ(&sems[sid].wait_q);
      EnQ(pid, &ready_q);
      pcbs[pid].state = READY;
   }
   else {
      sems[sid].sem_count++;
   }
}


