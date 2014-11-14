// irq7.c  Phase 5 of CpE/CSc 159
//
// the code needed for printing via parallel port

#include <spede/machine/parallel.h>    // for flag constants used below
#include "externs.h"   // has the printing semaphore ID needed in code
#include "sys_calls.h" // prototypes of sys calls
#include "isr.h"       // prototypes of ISR calls
#include "irq7.h"      // internal dependecies in this .c
#include "spede.h"     // cons_printf, inportb, outportb, IO_DELAY

int io_base = 0x378;   // io_base of parallel port 1 (LPT1)

// a driver process usually runs a service loop
void PrintDriver() {
   int result;
   msg_t msg;

// PrintInit() is optional for busy-poll printing
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

   print_sid = SemInit(-1); // why not 0? what happens if 0?

// init printer, wait (sleep), select and go:

// this resets the printer port, the connected printer will jitter
   outportb( io_base + LPT_CONTROL, PC_SLCTIN );
   status = inportb( io_base + LPT_STATUS );
   outportb( io_base + LPT_CONTROL, PC_INIT | PC_SLCTIN | PC_IRQEN );

   Sleep(1); // need time for printer to go thru its init stage

// print test message:
   print_result = PrintStr("Testing, 1-2-3-1-2-3...\n\n\0");
   if(print_result == -1) cons_printf( "PrintInit: test error!\n" );
}

// loop for each character in str (until null-terminated),
// calling PrintChar(). If a char in str is '\n' then also PrintChar('\r')
int PrintStr(char *str) {
   int TIME_OUT = 3 * 1666000;           // 3-sec time-out period
   int i, the_code, result;
   char ch;

   for(i = 0; i < TIME_OUT; i++) {
      the_code = inportb(io_base + LPT_STATUS) & PS_BUSY;
      if(the_code != 0) break;        // not busy, start print str!
      IO_DELAY();                     // .65 us
   }
   if(i == TIME_OUT) {  // timed out, return -1
      cons_printf(">>> PrintStr: printer timed out (power/online)?\n");
      return -1;
   }

   // then start printing the str:
   while( *str != '\0' ) { // not null, still more to print
      ch = *str;
      result = PrintChar( ch );
      if( result == -1 ) return -1; // return it from here, too

      if( ch == '\n' ) { // manual treatment of ASCII 10, add ASCII 13
         result = PrintChar( '\r' );
         if( result == -1 ) return -1; // return it from here, too
      }
      str++;
   }
   return 0; // all went fine
}

// See Tips.txt for details to implement a polled I/O version that
// that prints one character at a time. Then rewrite below to use
// interrupt-driven I/O.
int PrintChar(char ch) {
   //int TIME_OUT = 3*1666000;   // about 3 sec then time out
   int i, the_code;

   outportb(io_base + LPT_DATA, ch);          // send char as data
   the_code = inportb(io_base + LPT_CONTROL); // read control
   outportb(io_base + LPT_CONTROL, the_code | PC_STROBE); // strobe

   for(i=0; i<9; i++) IO_DELAY(); // Epson LQ571 needs 9

   outportb(io_base + LPT_CONTROL, the_code); // return control
// above outportb() causes an IRQ7 signal which is OK to still
// busy-poll for ACK below, unless IRQ7 has been enabled in printer
// port to trip CPU to look for ISR, then ACK won't be polled

// The following is busy-poll printer port for ACK of the char just
// sent. Experiment this then change to IRQ7 interrupt-driven method:
// replace code below (save return 0) with a single statement that
// starts with letter S.
//   for(i=0; i<TIME_OUT; i++) {
//      the_code = PS_ACK & inportb(io_base + LPT_STATUS);
//      if(the_code == 0) break; // printer ACK'ed
//      IO_DELAY(); // otherwise, wait 0.65 us, and loop
//   } 
//   if( i == TIME_OUT ) {
//      cons_printf(">>> PrintChar timed out!\n");
//      return -1; // timed out, return -1
//   }

   SemWait(print_sid);  // if using IRQ7 intr-driven I/O method
   return 0; // didn't time out/deadlock, return 0
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

