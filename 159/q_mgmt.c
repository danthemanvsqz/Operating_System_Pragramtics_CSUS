// q_mgmt.c, 159 1

#include "spede.h"    // need cons_printf()
#include "types.h"    // need q_t, Q_SIZE
#include "q_mgmt.h"
#include "util.h"

void MyBZero(char *p, int size){  while( size-- ) *p++ = 0;  }

void MyMemCpy(msg_t *dest, msg_t *src){ *dest = *src; }

int EmptyQ(q_t *p){ return(p->count == 0); } // empty when count is zero

int FullQ(q_t *p){ return(p->count == Q_SIZE); } // full when count is max possible

void InitQ(q_t *p){ p->count = p->head = p->tail = 0; } // reset the queue to empty, content not meaningful

void MyStrCpy(char *dest, char *src)
{
	int i=0;
	for(;src[i]!=0;i++)
		dest[i] = src[i];
	dest[i]=0;
}

int DeQ(q_t *p) // return -1 if queue is empty
{
   int id;

   if(EmptyQ(p))
   {
      cons_printf("Queue is empty, can't dequeue!\n");
      return -1;
   }
   id = p->q[p->head];
   p->head++;
   if(p->head == Q_SIZE) p->head = 0;   // wrap around
   p->count--;

   return id;
}

void EnQ(int id, q_t *p) // append id to p->q[tail] if queue has space
{
   if(FullQ(p))
   {
      cons_printf("Queue is full, can't enqueue!\n");
      return;
   }
   p->q[p->tail] = id;
   p->tail++;
   if(p->tail == Q_SIZE) p->tail = 0;   // wrap around
   p->count++;
}


int MsgFullQ(msg_q_t *q){ return (q->count == NUM_MSG); }

int MsgEmptyQ(msg_q_t *q) {  return (q->count == 0); }

void MsgEnQ(msg_t *p, msg_q_t *q)
{
	msg_t *slot = &q->msgs[q->tail];
	if(MsgFullQ(q))
	{
		cons_printf("Message queue is full, can't enqueue!\n");
		return;
	}
	MyMemCpy(slot, p);
	//MyBZero((char *) p, sizeof(msg_t));
	q->tail++;
	if(q->tail == NUM_MSG) q->tail = 0;   // wrap around
	q->count++;
}

void MsgDeQ(msg_t *p, msg_q_t *q)
{

	if(MsgEmptyQ(q))
	{
		cons_printf("Message queue is empty, can't dequeue!\n");
		return;
	}
	
	MyMemCpy(p,  &(q->msgs[q->head]));
	MyBZero((char *) &(q->msgs[q->head]), sizeof(msg_t));
	q->head++;
	if(q->head == NUM_MSG) q->head = 0;   // wrap around
	q->count--;
}


int CharEmptyQ(char_q_t *q){ return(q->count == 0); }

int CharFullQ(char_q_t *q){ return(q->count == CHAR_Q_SIZE); }

void CharInitQ(char_q_t *q){ q->count = q->head = q->tail = 0; }

char CharDeQ(char_q_t *q)
{
	char c;

   if(CharEmptyQ(q))
   {
      cons_printf("Character queue is empty, can't dequeue!\n");
      return -1;
   }
   c = q->buffer[q->head];
   q->head++;
   if(q->head == CHAR_Q_SIZE) q->head = 0;   // wrap around
   q->count--;

   return c;
}

void CharEnQ(char c, char_q_t *q)
{
	if(CharFullQ(q))
   {
      cons_printf("Character queue is full, can't enqueue!\n");
      return;
   }
   q->buffer[q->tail] = c;
   q->tail++;
   if(q->tail == CHAR_Q_SIZE) q->tail = 0;   // wrap around
   q->count++;
}



