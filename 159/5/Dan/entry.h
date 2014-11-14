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
#define MSGSND_INTR 54
#define MSGRCV_INTR 55
#define IRQ7_INTR 0x27 // Centronics (TM) parallel, LPT1 (PC DOS)

#define SEL_KCODE 8     // kernel's code segment
#define SEL_KDATA 16    // kernel's data segment
#define KERNEL_STACK_SIZE 8192  // kernel's stack size, in chars

// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "types.h" // for tf_t below

void TimerEntry();     // code defined in entry.S
void Loader(tf_t *);
void GetPidEntry();
void SleepEntry();
void SpawnEntry();
void SemInitEntry();
void SemWaitEntry();
void SemPostEntry();
void MsgSndEntry();
void MsgRcvEntry();
void IRQ7Entry();

__END_DECLS

#endif

#endif
