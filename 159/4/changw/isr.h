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
void MsgSndISR(int, msg_t*);
void MsgRcvISR(int, msg_t*);

void WakieWakie();  // called by TimerISR, wake-up sleepers
//void MyBZero(char *, int);
//void MyMemCpy(char *, char *, int);

#endif
