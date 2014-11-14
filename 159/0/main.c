/*******************************************************************************
	main.c Timer Lab
*******************************************************************************/

#include <spede/stdio.h>
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/proc_reg.h>
#include <spede/machine/seg.h>
#include <spede/machine/pic.h>

#include "entry.h"

typedef void (* func_ptr_t)();

struct i386_gate *idt_table;

void SetIDTEntry(int entry_num, func_ptr_t entry_addr){
	struct i386_gate *gateptr = &idt_table[entry_num];
	fill_gate(gateptr, (int) entry_addr, get_cs(), ACC_INTR_GATE, 0);
}

main(){
	int i;
	idt_table = get_idt_base();
	cons_printf("IDT is at memory location %u.\n", idt_table);
	SetIDTEntry(32, TimerEntry);
	outportb(0x21, ~0x01);
	EI();
	while(1){
		for(i=0; i<1666000; i++) IO_DELAY();
		cons_putchar('.');
		if(cons_kbhit()) break;
	}
}
