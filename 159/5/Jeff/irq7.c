// irq7.c  Phase 5 of CpE/CSc 159
//
// the code needed for printing via parallel port

#include <spede/machine/parallel.h>    // for flag constants used below
#include "externs.h"   // has the printing semaphore ID needed in code
#include "sys_calls.h" // prototypes of sys calls
#include "isr.h"       // prototypes of ISR calls
#include "irq7.h"      // internal dependecies in this .c
#include "spede.h"

int io_base = 0x378;   // io_base of parallel port 1 (LPT1)

// a driver process usually runs a service loop
void PrintDriver() {
   int result;
   msg_t msg;
   //msg.bytes = "Hello from the A-Team\n";
   PrintInit();     // get print_sid, reset/init printer port

   while(1)         // loop for print service
   {
      MsgRcv(&msg); // receive print request
      result = PrintStr(msg.bytes); // msg.byte is str to print
      if(result == -1)  // something timed out from PrintStr()
         cons_printf("\n >>> PrintDriver: print error!\n");
   }
}

// initialize data structures and the printer. Since there is only one
// printer device, we can use global variables. Resets the printer by
// setting control register without PC_INIT, then putting it back in.
void PrintInit() {
   int status, print_result;

   print_sid = SemInit(-1); // why -1, not 0? what happens if 0?

// init printer, wait (sleep), select and go:

//????????????????????????? CODE BELOW ???????????????????????????
// this resets the printer port, the connected printer will jitter
// outportb PC_SLCTIN to io_base + LPT_CONTROL
// read status from io_base + LPT_STATUS
   outportb(io_base + LPT_CONTROL, PC_SLCTIN);
   status = inportb(io_base + LPT_STATUS);
//????????????????????????? CODE ABOVE ???????????????????????????

   Sleep(1); // need time for printer to go thru its init stage

//????????????????????????? CODE BELOW ???????????????????
// outportb or-ed flags: PC_INIT | PC_SLCTIN | PC_IRQEN to
// io_base + LPT_CONTROL
   outportb(io_base + LPT_CONTROL, PC_INIT | PC_SLCTIN | PC_IRQEN);
//????????????????????????? CODE ABOVE ??????????????????????
// above has a side effect of brining printer port to trigger
// IRQ7ISR immediately (busy-poll printing need not do this):

//????????????????????????? CODE BELOW ??????????????????????
// print test message:
// print_result = PrintStr("Testing, 1-2-3-1-2-3...\n\n\0");
// if print_result is -1, show "PrintInit: test error!" on the PC
//cons_printf("Printing\n");
   print_result = PrintStr("Testing, 1-2-3-1-2-3...\n\n\0");
   if(print_result == -1) {
      cons_printf("PrintInit: test error!\n");
   }
   //cons_printf("Done [%d]\n", print_result);
//????????????????????????? CODE ABOVE ???????????????????????????
}

// loop for each character in str (until null-terminated),
// calling PrintChar(). If a char in str is '\n' then also PrintChar('\r')
int PrintStr(char *str) {
   //int result;
   //char ch;

   //????????????????????????? CODE BELOW ?????????????????????
   // add the printer-test code to see if the printer is online
   // see description in Tips.txt
   int TIME_OUT = 3 * 1666000;           // 3-sec time-out period
   int i, the_code;//result;
   //char ch;

   for(i = 0; i < TIME_OUT; i++) {
      the_code = inportb(io_base + LPT_STATUS) & PS_BUSY;
      if(the_code != 0) break;        // not busy, start print str!
      IO_DELAY();                     // .65 us
   }
   if(i == TIME_OUT) {  // timed out, return -1
      cons_printf(">>> PrintStr: printer timed out (power/online)?\n");
      return -1;
   }
   //cons_printf("PrintChar\t");
   for(i = 0; str[i] != 0; i++){
      //cons_printf("%c", str[i]);
      if(PrintChar(str[i]) == -1) return -1;
      if(str[i] == '\n') {
         if(PrintChar('\r') == -1) return -1;
      }
   }
   return 0;
   //????????????????????????? CODE ABOVE ?????????????????????

   //???????????????????????? CODE BELOW ???????????????????????????
   // for each ch of str: (convert this line to code, can use str++)
   //    send it to PrintChar()
   //    if PrintChar() returns -1, return that (failed to print ch)
   //    if ch is '\n' (manual treatment of ASCII 10, add ASCII 13),
   //    also prints char '\r' which is printer's newline char
   //    if returned -1 return that
   // end loop
   //???????????????????????? CODE ABOVE ???????????????????????????

   //result = 0;                // all results OK
}

// See Tips.txt for details to implement a polled I/O version that
// that prints one character at a time. Then rewrite below to use
// interrupt-driven I/O.
int PrintChar(char ch) {
   //?????????????????????? CODE BELOW ?????????????????????????
   // 1. assume printer's ready, strobe out the data (Tips.txt)
   // 2. loop/busy-poll for ACK signal from port until timed out
   // 3. if working, proceeds to try the "better" I/O method
   int TIME_OUT = 3*1666000;   // about 3 sec then time out
   int i = 0, the_code;
   //cons_printf("%c", ch);
   
   outportb(io_base + LPT_DATA, ch);          // send char as data
   the_code = inportb(io_base + LPT_CONTROL); // read control
   outportb(io_base + LPT_CONTROL, the_code | PC_STROBE); // strobe
   IO_DELAY(); IO_DELAY(); IO_DELAY();        // small delay
   outportb(io_base + LPT_CONTROL, the_code); // return control
   //?????????????????????? CODE ABOVE ?????????????????????????
   
   for(; i < TIME_OUT; i++){
      the_code = PS_ACK & inportb(io_base + LPT_STATUS);
      if(the_code == 0) break;
      IO_DELAY();
   }
   if(i >= TIME_OUT) {
      cons_printf(">>> PrintChar timed out!\n");
      return -1;
   }
   return 0;
}

// this IRQ7 handler can move to isr.c/isr.h
//
// handle interrupt from port, when this occurs the char just
// sent has been taken by printer to print, can now do next char
//
void IRQ7ISR() {
   SemPostISR(print_sid); // perform SemPostISR directly from here
   outportb(0x20, 0x67);  // 0x20 is PIC control, 0x67 dismisses IRQ 7
}

