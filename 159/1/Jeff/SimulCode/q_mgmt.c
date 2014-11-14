// q_mgmt.c, 159

#include "spede.h"
#include "types.h"
#include "externs.h"

int EmptyQ(q_t *p)
{
   return (p->count == 0);
}

int FullQ(q_t *p)
{
	return (p->count == Q_SIZE);
}

void InitQ(q_t *p)
{
	unsigned long i=0;
	
	p->count = p->head = p->tail = 0;
	for(;i<Q_SIZE;i++) p->q[i] = 0;
}

int DeQ(q_t *p) // return -1 if q is empty
{
	int pid;
	
	if(EmptyQ(p))
		return -1;
	
	p->count--;
	pid = p->q[p->head];	
	p->head = ++p->head % Q_SIZE;
	return pid;
}

void EnQ(int element, q_t *p)
{
	if(FullQ(p))
		return;
	
	p->count++;
	p->q[p->tail] = element;
	p->tail = ++p->tail % Q_SIZE;
}
