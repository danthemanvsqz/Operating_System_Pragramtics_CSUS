// q_mgmt.c, 159 1

#include "spede.h"    // need cons_printf()
#include "types.h"    // need q_t, Q_SIZE

int EmptyQ(q_t *p) // empty when count is zero
{
   return(p->count == 0);
}

int FullQ(q_t *p) // full when count is max possible
{
   return(p->count == Q_SIZE);
}

void InitQ(q_t *p) // reset the queue to empty, content not meaningful
{
   p->count = p->head = p->tail = 0; // or: bzero(p, sizeof(q_t));
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
