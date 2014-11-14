// q_mgmt.c, 159

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "q_mgmt.h"

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
	p->head = p->tail = p->count = 0;
}

int DeQ(q_t *p) // return -1 if q is empty
{
   int pid;

	if(p->count == 0)
		return -1;
	pid = p->q[p->head];
	p->count--;
	p->head = (p->head + 1) % Q_SIZE;

   return pid;
}

void EnQ(int element, q_t *p)
{
	p->count++;
	p->tail = (p->head + p->count - 1) % Q_SIZE;
	p->q[p->tail] = element;
}

int getPID(int index, q_t *p) {
	
	int position = (p->head + index) % Q_SIZE;
	return p->q[position];
}

void printQ(q_t *p) {
	int i = 0;
	//int pid;
	for(; i < p->count; i++) {
		//pid = getPID(i, p);
		//cons_printf("%i\n", pid);
		cons_printf("%i\n", getPID(i, p));
	}
}

