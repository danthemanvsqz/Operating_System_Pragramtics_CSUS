#include "util.h"
#include "types.h"
#include "spede.h"
#include "externs.h"


void PrintMsg(msg_t *p)
{
	int i;
	
	cons_printf("\nSender: %i\nSend Tick: %i\nNumbers: ", p->sender, p->send_tick);
	for(i=0;i<NUM_NUM;i++)
		cons_printf("%i ", p->numbers[i]);
	cons_printf("\nBytes: ");
	for(i=0; (i < NUM_BYTE) && (p->bytes[i] != 0); i++)
		cons_printf("%c", p->bytes[i]);
	cons_printf("\n");
}


void PrintMsgQ(int index) // TODO remove
{
	
	int i=-1;
	msg_q_t *q = &mboxes[index].msg_q;
	msg_t *msg = q->msgs;
	while ((++i < q->count) && (i<20))
		cons_printf(	"From:%02i, Tick:%02i, Num[0]: %i\n",
						msg[i%NUM_MSG].sender,
						msg[i%NUM_MSG].send_tick,
						msg[i%NUM_MSG].numbers[0]);
}
