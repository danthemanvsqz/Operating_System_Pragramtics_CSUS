#include "memory.h"
#include "types.h"
#include "externs.h"

//This function searches the table of spaces and returns the ID (index) of
// an available space, or returns -1 if there are no available spaces.

int getAvailableSpace() {
	int err = -1;
   int i=0;
	for (; i<NUM_PAGE; i++) {
		if (pages[i].available) {
			pages[i].available = 0;
			return i;
		}
	}
	return err;
}

void addressToWord(char *dest, char *src){
   int addr_trans = (int)src;
   dest[3] = (char) (addr_trans & 0x000000FF);
   dest[2] = (char) ((addr_trans & 0x0000FF00) >> 8);
   dest[1] = (char) ((addr_trans & 0x00FF0000) >> 16);
   dest[0] = (char) ((addr_trans & 0xFF000000) >> 24);
}

