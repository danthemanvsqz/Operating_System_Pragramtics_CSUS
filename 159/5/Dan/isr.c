// isr.c, 159 1
// ISR's called from Kernel()

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "q_mgmt.h"
#include "isr.h"
#include "proc.h"

void SpawnISR(int pid, func_ptr_t addr)
{
   MyBZero(user_stacks[pid], USER_STACK_SIZE);    // clean runtime stack
   MyBZero((char *)&mboxes[pid], sizeof(mbox_t)); // clear its private mbox

// 1st. point to just above of user stack, then drop by 64 bytes (tf_t)
   pcbs[pid].tf_p = (tf_t *)&user_stacks[pid][USER_STACK_SIZE];
   pcbs[pid].tf_p--;    // pointer arithmetic, now points to trapframe

// fill in CPU's register context
   pcbs[pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
   pcbs[ pid ].tf_p->eip = (int)addr; // new proc inst addr
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

   WakieWakie();

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

void WakieWakie() // wake if (multiple) processes to wake
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

// update sleep_q with the now ordered tmp_q
   sleep_q = tmp_q; // assignment allowed for struct type

   pcbs[cur_pid].state = SLEEP;
   cur_pid = -1; // need to get a different proc to run now
}

void SemInitISR()
{
   int sem_count = pcbs[ cur_pid ].tf_p->eax;
   int sid = DeQ( &avail_sem_q ); // get sid and put in ebx can be in Kernel()

   if(sid != -1)
   {
      MyBZero( (char *)&sems[ sid ], sizeof( sem_t ) );
      sems[ sid ].sem_count = sem_count;
   }
   pcbs[ cur_pid ].tf_p->ebx = sid;
   
}

void SemWaitISR( int sid ) // passing arg as device driver phase uses this
{
   if( sems[ sid ].sem_count > 0)
   {
      sems[ sid ].sem_count--;
   }
   else
   {
      pcbs[ cur_pid ].state = WAIT;
      EnQ( cur_pid, &sems[ sid ].wait_q );
      cur_pid = -1;
   }
}

void SemPostISR( int sid ) // passing arg as device driver phase uses this
{
   if( EmptyQ( &sems[ sid ].wait_q ) )
   {
      sems[ sid ].sem_count++;
   }
   else
   {
      int pid = DeQ( &sems[ sid ].wait_q );
      pcbs[ pid ].state = READY;
      EnQ( pid, &ready_q );
   }
}

void MsgSndISR() 
{	
   int mid, pid;
   msg_t *src, *dest;
   mid = pcbs[cur_pid].tf_p->eax;
   src = (msg_t *)pcbs[cur_pid].tf_p->ebx;

   src->sender = cur_pid;      // authenticate sender
   src->time_stamp = sys_tick; // authenticate time_stamp

   if(EmptyQ(&mboxes[mid].wait_q))
   { 
      MsgEnQ( src, &( mboxes[mid].msg_q ) );
   }
   else
   {	
      pid = DeQ(&mboxes[mid].wait_q);
      pcbs[pid].state = READY;
      EnQ(pid, &ready_q);

      dest = (msg_t *)pcbs[pid].tf_p->eax;
      *dest = *src;
   }
}

void MsgRcvISR()
{
   //int mid;
   msg_t *dest, *src;

   //mid = pcbs[cur_pid].tf_p->eax;
   dest = (msg_t *)pcbs[cur_pid].tf_p->eax;
	
   if(MsgEmptyQ(&mboxes[cur_pid].msg_q))
   { 
      EnQ(cur_pid, &mboxes[cur_pid].wait_q);
      pcbs[cur_pid].state = WAIT;
      cur_pid = -1;
   }
   else
   {	
      src = MsgDeQ(&mboxes[cur_pid].msg_q);
      *dest = *src;
   }
}

