// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 32
#define GETPID_INTR 48
#define SLEEP_INTR 49
#define SPAWN_INTR 50
#define SEMINIT_INTR 51
#define SEMWAIT_INTR 52
#define SEMPOST_INTR 53

#define SEL_KCODE 8     // kernel's code segment
#define SEL_KDATA 16    // kernel's data segment
#define KERNEL_STACK_SIZE 8192  // kernel's stack size, in chars


// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "types.h" // for tf_t below

extern void TimerEntry();     // code defined in entry.S
extern void Loader(tf_t *);
extern void GetPidEntry();
extern void SleepEntry();
extern void SpawnEntry();
extern void SemInitEntry();
extern void SemWaitEntry();
extern void SemPostEntry();

__END_DECLS

#endif

#endif
