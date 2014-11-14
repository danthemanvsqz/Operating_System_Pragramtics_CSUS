// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 32

#define SEL_KCODE 8             // kernel's code segment
#define SEL_KDATA 16            // kernel's data segment
#define KERNEL_STACK_SIZE 8192  // kernel's stack size, in chars
#define GETPID_INTR 48
#define SLEEP_INTR 49

// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "types.h" // for tf_t below

extern void TimerEntry();     // code defined in entry.S
extern void Loader(tf_t *);   // code defined in entry.S
extern void GetPidEntry();
extern void SleepEntry();

__END_DECLS

#endif

#endif
