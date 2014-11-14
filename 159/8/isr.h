// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "types.h" // func_ptr_t

void TimerISR();
void SleepISR();
void SpawnISR(int, func_ptr_t);
void SemInitISR();
void SemWaitISR();
void SemPostISR();
void MsgSndISR();
void MsgRcvISR();
void ForkISR(int);
void ExitISR();
void WaitISR();

void WakieWakie();  // called by TimerISR, wake-up sleepers

#endif
