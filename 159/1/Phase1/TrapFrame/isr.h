// isr.h CSC159

#ifndef _ISR_H_
#define _ISR_H_

#include "trap.h"

void SpawnISR(int);
void KillISR();
void ShowStatusISR();
void TimerISR();
void MyBzero(void *, int);
void printTrap(tf_t *);

#endif
