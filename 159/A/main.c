// Team Name: ???
// Members: ???, ???, ???
// main.c, 159

#include "spede.h"
#include "types.h"
#include "main.h"
#include "q_mgmt.h"
#include "proc.h"  // SimpleProc()
#include "entry.h" // TIMER_INTR 32 & addr of TimerEntry
#include "isr.h"
#include "irq7.h"  // IRQ7ISR() mentioned in main()
#include "irq34.h"

// globals:
struct i386_gate *idt_table;
int cur_pid, sys_tick, print_sid, OS_MT;
q_t ready_q, avail_q, sleep_q, avail_sem_q;
pcb_t pcbs[NUM_PROC];               // process table
char user_stacks[NUM_PROC][USER_STACK_SIZE]; // run-time stacks for processes
sem_t sems[NUM_SEM];
mbox_t mboxes[NUM_PROC];
page_t pages[NUM_PAGE];


void main()
{
   InitData();  // initialize needed data for kernel
   InitControl();
   SpawnISR(0, IdleProc); // create IdleProc to run if no user processes
   SpawnISR(1, Init); // create IdleProc to run if no user processes
   cur_pid = 0;
   Loader(pcbs[0].tf_p);
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
//   outportb(0x21, ~1);

   SetIDTEntry(39, IRQ7Entry);
   outportb(0x21, ~129); // for both IRQ 0 and 7

   SetIDTEntry(48, GetPidEntry);
   SetIDTEntry(49, SleepEntry);
   SetIDTEntry(50, SpawnEntry);
   SetIDTEntry(51, SemInitEntry);
   SetIDTEntry(52, SemWaitEntry);
   SetIDTEntry(53, SemPostEntry);
   SetIDTEntry(54, MsgSndEntry);
   SetIDTEntry(55, MsgRcvEntry);
   SetIDTEntry(0x23, IRQ3Entry);
   SetIDTEntry(0x24, IRQ4Entry);
   SetIDTEntry(56, ForkEntry);
   SetIDTEntry(57, WaitEntry);
   SetIDTEntry(58, ExitEntry);
   SetIDTEntry(0x0e, PFEntry);
   outportb(0x21, ~0x99);
}

void InitData()
{
   int i;
   char * nextFramePtr =
			(char *) ( (int) (_topHeapMemory + PAGE_SIZE)	//bump up
                           & ( ~(PAGE_SIZE-1) ) )  ;	//mask down

   OS_MT = get_cr3();
   InitQ(&avail_q); // set queues initially empty
   InitQ(&ready_q);
   InitQ(&sleep_q);
   InitQ(&avail_sem_q);

   for(i=2; i<NUM_PROC; i++) // skip 0 (IdleProc) and 1 (Init)
   {
      pcbs[i].state = AVAIL;
      pcbs[i].ppid = -1;
      EnQ(i, &avail_q);
   }
   for(i=NUM_SEM-1; i>=0; i--) EnQ(i, &avail_sem_q); // avail sem ID's 19-0
   
   for(i = 0; i < NUM_PAGE; i++){
      pages[i].available = 1;
      pages[i].addr = nextFramePtr;
      pages[i].pid = -1;
      nextFramePtr += PAGE_SIZE;
   }
}

void Scheduler() // this is kernel's process scheduler, simple round robin
{
   if(cur_pid > 0) return; // when cur_pid is not 0 (IdleProc) or -1 (none)

   if(cur_pid == 0) pcbs[0].state = READY; // skip when cur_pid is -1

   if(EmptyQ(&ready_q)) cur_pid = 0; // ready q empty, use IdleProc
   else cur_pid = DeQ(&ready_q);     // or get 1st process from ready_q

   pcbs[cur_pid].state = RUN;   // RUN state for newly selected process
   //set_cr3(pcbs[ cur_pid ].MT);
}

void Kernel(tf_t *tf_p) // kernel begins its control upon/after interrupt
{
   int pid;

   pcbs[ cur_pid ].tf_p = tf_p; // save for cur_pid which may change

   switch( tf_p->intr_id )
   {
      case TIMER_INTR:
         TimerISR();
         break;
      case IRQ7_INTR:
         IRQ7ISR(); // service parallel printer port event
         break;
      case GETPID_INTR:
         tf_p->eax = cur_pid;
         break;
      case SLEEP_INTR:
         SleepISR();
         break;
      case SPAWN_INTR:
         pid = DeQ( &avail_q ); // get pid and put in ebx can be in Kernel()
         pcbs[ cur_pid ].tf_p->ebx = pid; // child PID, could be -1
         if( pid == -1 ) break; // no more process
         SpawnISR(pid, (func_ptr_t) pcbs[ cur_pid ].tf_p->eax);
         break;
      case SEMINIT_INTR:
         SemInitISR();
         break;
      case SEMWAIT_INTR:
         SemWaitISR(tf_p->eax);
         break;
      case SEMPOST_INTR:
         SemPostISR(tf_p->eax);
         break;
      case MSGSND_INTR:
         MsgSndISR();
         break;
      case MSGRCV_INTR:
         MsgRcvISR();
         break;
      case IRQ3_INTR:	
      case IRQ4_INTR:
         IRQ34ISR();	
         break;
      case FORK_INTR:
         pid = DeQ( &avail_q );
         ForkISR(pid);
         pcbs[ cur_pid ].tf_p->ebx = pid;
         break;
      case WAIT_INTR: //cons_printf("Wait Intr\n");
         WaitISR(); 
         break;
      case EXIT_INTR: //cons_printf("Exit Intr\n");
         ExitISR();
         break;
      case PF_INTR:
         PFISR();
         break;
   }
   Scheduler();                  // find same/new process to run
   set_cr3(pcbs[ cur_pid ].MT);
   Loader(pcbs[ cur_pid ].tf_p); // load it to run
} // Kernel()

