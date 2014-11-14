// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_

#include "types.h" // for tf_t below

void main();
void InitData();
void InitControl();
void Scheduler();
void Kernel(tf_t *);

#endif
