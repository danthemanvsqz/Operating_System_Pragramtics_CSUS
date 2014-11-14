/*******************************************************************************
	isr.c Timer Lab
*******************************************************************************/

#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

unsigned short *vidmem = (unsigned short *) 0xB8000 + 12 * 80 + 40;

#define ASCII_SPACE (0xf00 + ' ')

void TimerISR(){
	static int ascii_char = ASCII_SPACE;
	static int tick_count = 0;
	
	outportb(0x20, 0x60);

	if(++tick_count %50 == 0){
		*vidmem = ascii_char;
		ascii_char++;
		if(ascii_char == ASCII_SPACE + 95)
			ascii_char = ASCII_SPACE;
	}
}
