// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "types.h"

void TimerISR();
void SpawnISR(int, func_ptr_t);

void ShowStatusISR();
void ShowQ(q_t *);

void MyBZero(char *, int);
void SleepISR();

int SemInitISR(int);
void SemWaitISR();
void SemPostISR();

void MsgSndISR(int, msg_t *);
void MsgRcvISR(int, msg_t *);

#endif
