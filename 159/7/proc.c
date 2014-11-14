// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "sys_calls.h"
#include "proc.h"
#include "q_mgmt.h"  // MyStrCpy()
#include "irq7.h"    // PrintDriver()
#include "irq34.h"
#include "shell_cmds.h"
#include "filesys.h"
#include "op_codes.h"

void IdleProc() {
   int i;

   while(1) {
      cons_printf("0 ");
      for(i=0;i<166600;i++)IO_DELAY();
   }
}


void Init()
{
	int cpid, i, key, print_driver_pid, file_system_pid;
	msg_t my_msg;
	print_driver_pid = Spawn(PrintDriver); // spawn driver proc
	file_system_pid = Spawn(FileSys);
   for(i = 0; i < NUM_TERM; i++){
      cpid = Spawn(Shell);
      my_msg.nums[0] = cpid;
      my_msg.nums[1] = i;
      my_msg.nums[2] = print_driver_pid;
      my_msg.nums[3] = file_system_pid;
      MsgSnd(cpid, &my_msg);
   }

	for(;;){
		//cons_printf("%d ", GetPid());
		//for(i=0; i<83300; i++)IO_DELAY();
		key = cons_getchar();
		switch(key)
		{
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
   int term_num, stdin_pid, stdout_pid, print_driver_pid, 
		file_system_pid;
   char login[50], passwd[50], cmd_str[50];
   msg_t msg;
   MsgRcv(&msg); 
   msg.sender = msg.nums[0];
   term_num = msg.nums[1];
   print_driver_pid = msg.nums[2];
   file_system_pid = msg.nums[3];
   msg.nums[4] = ECHO_ON;
 
   
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
         msg.nums[4] = ECHO_OFF;
         MsgSnd(stdout_pid, &msg);
         MsgRcv(&msg);
         msg.nums[4] = ECHO_ON;
         
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
         
         if( StrCmpLen(cmd_str, "print", 5) == 1 ){
            ShellPrint(cmd_str, print_driver_pid, file_system_pid);
            MyStrCpy(msg.bytes, cmd_str);
            MsgSnd(stdout_pid, &msg);
            MsgRcv(&msg);            
         }
         else if( StrCmp(cmd_str, "bye") == 1){
            break;
         }
         else if( StrCmp(cmd_str, "\n")){
            continue;
         }
         else if(StrCmp(cmd_str, "help")){
            ShellHelp(stdout_pid);
         }
         else if(StrCmp(cmd_str, "who")){
            ShellWho(stdout_pid);
         }
         else if(StrCmpLen(cmd_str, "dir", 3)){
            ShellDir(cmd_str, stdout_pid,file_system_pid);            
         }
         else if(StrCmpLen(cmd_str, "type", 4)){
            ShellType(cmd_str, stdout_pid, file_system_pid);
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
