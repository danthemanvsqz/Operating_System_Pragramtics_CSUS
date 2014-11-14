// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "types.h"

void TimerISR();
void SpawnISR(int, func_ptr_t);

void SleepISR(); // just leave it in isr.c

void ShowStatusISR();

void ShowQ(q_t *);
void WakyWaky();  // called by TimerISR, wake-up sleepers
void MyBZero(char *, int);
void MyMemCpy(char *, char *, int);

int SemInitISR(int);
void SemWaitISR();
void SemPostISR();

#endif
