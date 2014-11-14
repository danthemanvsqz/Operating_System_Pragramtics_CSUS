// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_

#include "types.h"

void main();
void InitData();
void InitControl();
void Scheduler();
void Kernel();
void SetIDTEntry(int , func_ptr_t);

#endif
