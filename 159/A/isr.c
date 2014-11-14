// isr.c, 159 1
// ISR's called from Kernel()

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "q_mgmt.h"
#include "isr.h"
#include "proc.h"
#include "memory.h"
#include "sys_calls.h"

void WaitISR(){ //cons_printf("WaitISR\t");
   int i = 1; 
   for(; i <= NUM_PROC; i++){
      if(pcbs[i].ppid == cur_pid){ 
         if(pcbs[i].state == ZOMBIE){ //cons_printf("ZOMBIE\n");
            pcbs[cur_pid].tf_p->eax = pcbs[i].exit_code;
            pcbs[i].state = AVAIL;
            pcbs[i].ppid = -1;
            EnQ(i, &avail_q);
            pcbs[cur_pid].state = READY;
            EnQ(cur_pid, &ready_q);
            cur_pid = -1;
            return;          
         }
         else{ //cons_printf("Wait Child\n");
            pcbs[cur_pid].state = WAIT_CHILD;
            cur_pid = -1;
            return;
         }
      }
   }
}

void ExitISR(){ //cons_printf("ExitISR\t");
   int i;
   pcbs[cur_pid].exit_code = pcbs[cur_pid].tf_p->eax;
   set_cr3(OS_MT);
   if(pcbs[pcbs[cur_pid].ppid].state == WAIT_CHILD){ //cons_printf("WAIT CHILD\n");
      pcbs[pcbs[cur_pid].ppid].state = READY;
      EnQ(pcbs[cur_pid].ppid, &ready_q);
      pages[pcbs[cur_pid].page_num].available = 1;
      pages[pcbs[cur_pid].page_num].pid = -1;
      pcbs[pcbs[cur_pid].ppid].tf_p->eax = pcbs[cur_pid].exit_code;
      pcbs[cur_pid].state = AVAIL;
      pcbs[cur_pid].ppid = -1;
      EnQ(cur_pid, &avail_q);
      cur_pid = -1;
   }
   else{ //cons_printf("ZOMBIE\n");
      pcbs[cur_pid].state = ZOMBIE; 
      cur_pid = - 1;
   }
   for(i=0;i<NUM_PAGE;i++){
      if(pages[i].pid == cur_pid){
         pages[i].available = 1;
         pages[i].pid = -1;
         MyBZero(pages[i].addr, PAGE_SIZE);
      }
   }
}

void ForkISR(int child_pid){
   int cp, ct, sp, st, mt;
   int dir, pte, offset;

   func_ptr_t addr = (func_ptr_t)pcbs[cur_pid].tf_p->eax;
   MyBZero((char *)&mboxes[child_pid], sizeof(mbox_t));
     
   // Allocate Pages
   mt = getAvailableSpace();
   pages[mt].pid = child_pid;
   ct = getAvailableSpace();
   pages[ct].pid = child_pid;
   st = getAvailableSpace();
   pages[st].pid = child_pid;
   cp = getAvailableSpace();
   pages[cp].pid = child_pid;
   sp = getAvailableSpace();
   pages[sp].pid = child_pid;

   // Copy first 4 entries of the OS_MT to MT
   MyMemCpy((char*)pages[mt].addr, (char*)OS_MT, 16);

   // VSTART
   dir = (VSTART >> 22) ;
   pte = (VSTART >> 12) & 0x3ff;

   *(int*)(pages[mt].addr + 4*dir) = (int)pages[ct].addr | 3;
   *(int*)(pages[ct].addr + 4*pte) = (int)pages[cp].addr | 3;

   // Copy code
   MyMemCpy(pages[cp].addr, (char*) addr, PAGE_SIZE);

   // VEND
   dir = (VEND-64) >> 22;
   pte = ((VEND-64) >> 12) & 0x3ff;
   offset = (VEND-64) & 0xfff;

   *(int*)(pages[mt].addr + 4*dir) = (int)pages[st].addr | 3;
   *(int*)(pages[st].addr + 4*pte) = (int)pages[sp].addr | 3;

   // Begin copying tf_p
   pcbs[child_pid].tf_p = (tf_t*)(pages[sp].addr + offset);

   pcbs[child_pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
   pcbs[child_pid].tf_p->eip = VSTART;	 // Set to Virtual space
   pcbs[child_pid].tf_p->cs = get_cs();
   pcbs[child_pid].tf_p->ds = get_ds();
   pcbs[child_pid].tf_p->es = get_es();
   pcbs[child_pid].tf_p->fs = get_fs();
   pcbs[child_pid].tf_p->gs = get_gs();
   pcbs[child_pid].tf_p = (tf_t*)(VEND - 64);	// Set to Virtual space

   pcbs[child_pid].tick_count = pcbs[child_pid].total_tick_count = 0;
   pcbs[child_pid].ppid = cur_pid;
   pcbs[child_pid].MT = (int)pages[mt].addr;

   pcbs[child_pid].state = READY;
   EnQ(child_pid, &ready_q);
}

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
   pcbs[pid].MT = OS_MT;
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
   int mid, pid, src_flag = 0, dest_flag = 0;
   msg_t *src, *dest;
   if(pcbs[cur_pid].ppid > 0){
      src_flag = 1;
      set_cr3(pcbs[cur_pid].MT);
   }
   src = (msg_t *)pcbs[cur_pid].tf_p->ebx;
   src->sender = cur_pid;      // authenticate sender
   src->time_stamp = sys_tick; // authenticate time_stamp
   
   mid = pcbs[cur_pid].tf_p->eax;
   if(pcbs[mid].ppid > 0){
      dest_flag = 1;
      set_cr3(pcbs[mid].MT);
   }
   
   
   //if(cur_pid > 6) breakpoint();
//      cons_printf("MsgSnd: msg=%s\n",src->bytes);
   
   if(EmptyQ(&mboxes[mid].wait_q))
   {
      MsgEnQ( src, &( mboxes[mid].msg_q ) );
   }
   else
   {	
      pid = DeQ(&mboxes[mid].wait_q);
      pcbs[pid].state = READY;
      EnQ(pid, &ready_q);

      dest = (msg_t *)pcbs[pid].tf_p->eax; // eax since MsgRcv changed
      *dest = *src;
   }
   if(src_flag || dest_flag){
      set_cr3(OS_MT);
   }
}


void MsgRcvISR()
{
   int mid, flag = 0;
   msg_t *dest, *src;
   if(pcbs[cur_pid].ppid > 0){
      flag = 1;
      set_cr3(pcbs[cur_pid].MT);
   }

   mid = cur_pid; // private mbox only now
   dest = (msg_t *)pcbs[cur_pid].tf_p->eax; // eax since MsgRcv changed
	
   if(MsgEmptyQ(&mboxes[mid].msg_q))
   {
      EnQ(cur_pid, &mboxes[mid].wait_q);
      pcbs[cur_pid].state = WAIT;
      cur_pid = -1;
   }
   else
   {	
      src = MsgDeQ(&mboxes[mid].msg_q);
      *dest = *src;
   }
   if(flag){
      set_cr3(OS_MT);
   }
}

