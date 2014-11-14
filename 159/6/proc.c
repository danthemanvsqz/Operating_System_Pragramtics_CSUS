// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"
#include "proc.h"
#include "q_mgmt.h"  // MyStrCpy()
#include "irq7.h"    // PrintDriver()
#include "irq34.h"

void IdleProc() {
   int i;

   while(1) {
      cons_printf("0 ");
      for(i=0;i<1666000;i++)IO_DELAY();
   }
}


void Init()
{
	int cpid, i, key, print_driver_pid;
	msg_t my_msg;
	print_driver_pid = Spawn(PrintDriver); // spawn driver proc
	cpid = Spawn(Shell);
	my_msg.nums[0] = cpid;
	my_msg.nums[1] = next_terminal;
	my_msg.nums[2] = print_driver_pid;
	next_terminal = (++next_terminal) % NUM_TERM;
	MsgSnd(cpid, &my_msg);

	for(;;){
		cons_printf("%d ", GetPid());
		for(i=0; i<833000; i++)IO_DELAY();
		key = cons_getchar();
		switch(key)
		{
			case 't':
				cpid = Spawn(Shell);
				my_msg.nums[0] = cpid;
				my_msg.nums[1] = next_terminal;
				my_msg.nums[2] = print_driver_pid;
				next_terminal = (++next_terminal) % NUM_TERM;
				MsgSnd(cpid, &my_msg);
				break;
			case 'b':
			breakpoint();
			break;
			case 'q':
			exit(0);
			default:
				
		}

	}
   
}

void Shell()
{
   int term_num, stdin_pid, stdout_pid, print_driver_pid;
   char login[50], passwd[50], cmd_str[50];
   msg_t msg;
   MsgRcv(&msg);
   msg.sender = msg.nums[0];
   term_num = msg.nums[1];
   print_driver_pid = msg.nums[2];
   
   
   stdin_pid = Spawn(Stdin); 
   stdout_pid = Spawn(Stdout);	
   MsgSnd(stdin_pid, &msg);	
   MsgSnd(stdout_pid, &msg);	
   TerminalInit(term_num);   
   
   while(1){
      while(1){
         MyStrCpy(msg.bytes, "login : ");
         MsgSnd(stdout_pid, &msg);
         MsgRcv(&msg);
         
         
         MsgSnd(stdin_pid, &msg);
         MsgRcv(&msg);
         MyStrCpy(login, msg.bytes);
         
         MyStrCpy(msg.bytes, "password: ");
         MsgSnd(stdout_pid, &msg);
         MsgRcv(&msg);
         
         MsgSnd(stdin_pid, &msg);
         MsgRcv(&msg);
         MyStrCpy(passwd, msg.bytes);
         
         if(StrCmp(login, passwd) == 1) break;
         
         MyStrCpy(msg.bytes, "Illegal login and password!\n");
         MsgSnd(stdout_pid, &msg);
         MsgRcv(&msg);
      }
      
      while(1){
         MyStrCpy(msg.bytes, "\nOS Alpha > ");
         MsgSnd(stdout_pid, &msg);
         MsgRcv(&msg);
         
         MsgSnd(stdin_pid, &msg);
         MsgRcv(&msg);
         MyStrCpy(cmd_str, msg.bytes);
         
         if( StrCmp(cmd_str, "print") == 1 ){
            MyStrCpy(msg.bytes, "A Team\n");
            MsgSnd(print_driver_pid, &msg);
         }
         else if( StrCmp(cmd_str, "bye") == 1){
            break;
         }
         else if( StrCmp(cmd_str, "\n") || StrCmp(cmd_str, "\r") || 
                  StrCmp(cmd_str, "\n\r")){
            continue;
         }
         else {
            MyStrCpy(msg.bytes, "Invalid command!\n");
            MsgSnd(stdout_pid, &msg);
            MsgRcv(&msg);
         }
      }
   }
}


























//
