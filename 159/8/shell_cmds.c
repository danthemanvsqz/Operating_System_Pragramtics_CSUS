// shell_cmds.c, Phase 7, Sac State CpE/CSc 159
// shell subroutines to perform "dir/type/print" commands
// complete the pseudo code below, prototype them into "shell_cmds.h"

#include "spede.h"
#include "types.h"
#include "sys_calls.h"
#include "op_codes.h" // operation codes: OK, STAT, etc.
#include "filesys.h"
#include "externs.h"
#include "q_mgmt.h"   // MyStrCmp(), etc.
#include "shell_cmds.h"


// buld a string: a detail line of attributes of the given file/directory
// that p points to (stat_t)
void DirLine(stat_t *p, char *line) {
// we get back in msg.bytes two parts: stat_t type and path
// by type-casting "p+1" pointer, we get a pointer to the path
   char *path = (char *)(p + 1);

// build output in the line from what p points
   sprintf(line, "----  size=%5d     %s\n", p->size, path);

   if ( S_ISDIR(p->mode) ) line[0] = 'd';  // mode is directory
   if ( QBIT_ON(p->mode, S_IROTH) ) line[1] = 'r'; // mode is readable
   if ( QBIT_ON(p->mode, S_IWOTH) ) line[2] = 'w'; // mode is writable
   if ( QBIT_ON(p->mode, S_IXOTH) ) line[3] = 'x'; // mode is executable
}


// do "dir" listing, Shell() communicates with FileSys() and Stdout()
// make sure str passed has trailing 0: "dir\0" or "dir something\0"
void ShellDir(char *str, int stdout, int filesys) {
   char path[NUM_BYTE], line[NUM_BYTE];
   stat_t *p;
   msg_t msg;

// if str is "dir\0" assume home (root) dir "/"
// else, assume user specified a path after first 4 letters "dir "
   if(StrCmp(str, "dir\0") == 1) {
      path[0] = '/';
      path[1] = '\0'; // null-terminate the path[]
   } 
   else { // skip 1st 4 letters "dir " and get the path
      str += 4;
      MyStrCpy(path, str); // make sure str is null-terminated from Shell()
   }

   MyMemCpy(msg.bytes, path, sizeof(path));
   msg.nums[0] = STAT;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
   if(msg.nums[0] != OK) {
      MyStrCpy(msg.bytes, "Error: ");
      sprintf(&msg.bytes[7], "%d", msg.nums[0]);
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;        // we can't continue
   }

// By op_code STAT, file sys returns in msg.bytes a "stat_t" type,
// take a peek, if user directed us to a file, then display info for only
// that file; otherwise, it's a directory, needs to display each entry in
// the dir content as if we are "listing" it
   p = (stat_t *)msg.bytes;    // p, status type pointer

   if( ! S_ISDIR(p->mode) ) // if not dir, it's a file, detail-list it
   {
      DirLine(p, line); // line is to be built and returned by subroutine
      MyStrCpy(msg.bytes, line);
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;     // we can't continue
   }
// it's a dir, we list its content (entries), one by one in a loop
// we 1st request FileSys to open the dir, and then issue a read on
// each entry in it in the loop
   MyStrCpy(msg.bytes, path);
   msg.nums[0] = OPEN;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);

   while(msg.nums[0] == OK) {
      msg.nums[0] = READ;
      MsgSnd(filesys, &msg);
      MsgRcv(&msg);
      p = (stat_t *)msg.bytes;
      DirLine(p, line); // convert msg.bytes into a detail line
      MyStrCpy(msg.bytes, line);
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
   }
   msg.nums[0] = CLOSE;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
}

void ShellExec(char *cmd_str, int stdout, int filesys){
   //char path[NUM_BYTE];
   stat_t *p;
   msg_t msg;
   int child_pid, exit_code;
   MyStrCpy(msg.bytes, cmd_str);
   msg.nums[0] = STAT;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
   if(msg.nums[0] != OK){
      MyStrCpy(msg.bytes, "Error: ");
      sprintf(&msg.bytes[7], "%d", msg.nums[0]);
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;
   }
   p = (stat_t *)msg.bytes;    // p, status type pointer
   if( p->mode != MODE_EXEC ) {    // if dir, can't do it
      MyStrCpy(msg.bytes, "Error: Invalid File Type\n");
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;
   } 
   child_pid = Fork(p->content);
   if(child_pid == -1){
      MyStrCpy(msg.bytes, "Error no more processes available\n");
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;
   }
   sprintf(msg.bytes, "Child Forked, PID: %d\n", child_pid);
   MsgSnd(stdout, &msg);
   MsgRcv(&msg);
   MyBZero((char*)&msg, sizeof(msg_t));
   msg.nums[0]=stdout;
   MsgSnd(child_pid, &msg);
   exit_code = Wait();
   sprintf(msg.bytes, "Child Exited, exit code: %d %x\n", exit_code, exit_code); 
   MsgSnd(stdout, &msg); 
   MsgRcv(&msg);
}


void ShellType(char *str, int stdout, int filesys) {
   stat_t *p;
   msg_t msg;
   char path[NUM_BYTE];
//*************************************************************************
// write code:
// if str is "type\0" display "Usage: type <filename>\n\0"
//    return;                  // can't continue
   if(StrCmp(str, "type") == 1){
      MyStrCpy(msg.bytes, "Usage: type <filename>\n\0");
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;
   }
// skip 1st 5 characters in str ("type ") to get the rest (str)
   MyStrCpy(path, &str[5]);
// copy str to msg.bytes
   MyStrCpy(msg.bytes, path);
// ask FileSys to STAT this
   msg.nums[0] = STAT;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
// if result NOT OK, display an error msg...
//    return;         // we can't continue
   if(msg.nums[0] != OK){
      MyStrCpy(msg.bytes, "Error: ");
      sprintf(&msg.bytes[7], "%d", msg.nums[0]);
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;
   }
//
// By op_code STAT, file sys returns in msg.bytes a "stat_t" type,
// take a peek, if user directed us to a file, then display info for only
// that file; otherwise, it's a directory, display an error msg to user.
//
   p = (stat_t *)msg.bytes;    // p, status type pointer
   if( S_ISDIR(p->mode) ) {    // if dir, can't do it
      MyStrCpy(msg.bytes, "Error: Invalid File Type\n");
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
      return;
   }
//*************************************************************************

//*************************************************************************
// write code:
// request FileSys to OPEN (prep msg, send FileSys)
// then start READ loop (until NOT OK)
   msg.nums[0] = OPEN;
   MyStrCpy(msg.bytes, path);
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
   while(msg.nums[0] == OK) {         // so long OK
//    request FileSys to READ (prep msg, send FileSys)
      msg.nums[0] = READ;
      MsgSnd(filesys, &msg);
//    receive back in msg, text in msg.bytes
      MsgRcv(&msg);
      if(msg.nums[0] != OK) break;
//
//    send it to Stdout to show it to terminal
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
//    receive synchro msg back
   }
//*************************************************************************

//*************************************************************************
// write code:
// request FileSys to close
// if result NOT OK, display an error msg...
//*************************************************************************
   msg.nums[0] = CLOSE;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
   if(msg.nums[0] != OK){
      MyStrCpy(msg.bytes, "Error: ");
      sprintf(&msg.bytes[7], "%d", msg.nums[0]);
      MsgSnd(stdout, &msg);
      MsgRcv(&msg);
   }
}

//*************************************************************************
// write code:
// Other shell commands...
//*************************************************************************

void ShellPrint(char *str, int print_driver_pid, int filesys){
   char path[NUM_BYTE];
   stat_t *p;
   msg_t msg;

//*************************************************************************
// write code:
// if str is "type\0" display "Usage: type <filename>\n\0"
//    return;                  // can't continue
   if(StrCmp(str, "print") == 1){
      MyStrCpy(str, "Usage: print <filename>\n\0");
      return;
   }
// skip 1st 5 characters in str ("type ") to get the rest (str)
   MyStrCpy(path, &str[6]);
// copy str to msg.bytes
   MyStrCpy(msg.bytes, path);
// ask FileSys to STAT this
   msg.nums[0] = STAT;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
// if result NOT OK, display an error msg...
//    return;         // we can't continue
   if(msg.nums[0] != OK){
      MyStrCpy(msg.bytes, "Error: ");
      sprintf(&msg.bytes[7], "%d", msg.nums[0]);
      MyStrCpy(str, msg.bytes);
      return;
   }
//
// By op_code STAT, file sys returns in msg.bytes a "stat_t" type,
// take a peek, if user directed us to a file, then display info for only
// that file; otherwise, it's a directory, display an error msg to user.
//
   p = (stat_t *)msg.bytes;    // p, status type pointer
   if( S_ISDIR(p->mode) ) {    // if dir, can't do it
      MyStrCpy(msg.bytes, "Error: Invalid File Type\n");
      MyStrCpy(str, msg.bytes);
      return;
   }
//*************************************************************************

//*************************************************************************
// write code:
// request FileSys to OPEN (prep msg, send FileSys)
// then start READ loop (until NOT OK)
   msg.nums[0] = OPEN;
   MyStrCpy(msg.bytes, path);
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
   while(msg.nums[0] == OK) {         // so long OK
//    request FileSys to READ (prep msg, send FileSys)
      msg.nums[0] = READ;
      MsgSnd(filesys, &msg);
//    receive back in msg, text in msg.bytes
      MsgRcv(&msg);
      if(msg.nums[0] != OK) break;
//
//    send it to Stdout to show it to terminal
      MsgSnd(print_driver_pid, &msg);
      MyStrCpy(str, "File sent to printer\n");
//    receive synchro msg back
   }
//*************************************************************************

//*************************************************************************
// write code:
// request FileSys to close
// if result NOT OK, display an error msg...
//*************************************************************************
   msg.nums[0] = CLOSE;
   MsgSnd(filesys, &msg);
   MsgRcv(&msg);
   if(msg.nums[0] != OK){
      MyStrCpy(msg.bytes, "Error: ");
      sprintf(&msg.bytes[7], "%d", msg.nums[0]);
      MyStrCpy(str, msg.bytes);
   }
}
void ShellHelp(int stdout_pid){
   msg_t help_msg;
   MyStrCpy(help_msg.bytes, 
   "Available commands are:\nhelp, who, dir <name>, type <name>, print <name>, and bye. Good luck!\n");
   MsgSnd(stdout_pid, &help_msg);
   MsgRcv(&help_msg);
}

void ShellWho(int stdout_pid){
   msg_t msg;
   MyStrCpy(msg.bytes, "A Team\n");
   MsgSnd(stdout_pid, &msg);
   MsgRcv(&msg);
}
