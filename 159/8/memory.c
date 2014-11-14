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

