// q_mgmt.c, 159 1

#include "spede.h"    // need cons_printf()
#include "types.h"    // need q_t, Q_SIZE



// code your own bzero() & memcpy() of C Run-Time Lib (CRTL)
void MyBZero(char *p, int size)
{
   while( size-- ) *p++ = 0; // they'll be all nulls
}

void MyMemCpy(char *dest, char *src, int size)
{
   while(size--) *dest++ = *src++;
}

void MyStrCpy(char *dest, char *src){
   while((*dest++ = *src++));
}

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

int MsgFullQ(msg_q_t *p){
   return p->count == NUM_MSG;
}

int MsgEmptyQ(msg_q_t *p){
   return p->count == 0;
}

void MsgEnQ(msg_t *p, msg_q_t *q){
   if(MsgFullQ(q))
   {
      cons_printf("Queue is full, can't enqueue!\n");
      return;
   }
   MyMemCpy((char*) &(q->msgs[q->tail]), (char*) p, sizeof(msg_t));
   
   q->tail++;
   if(q->tail == NUM_MSG) q->tail = 0;
   q->count++;
}


msg_t *MsgDeQ(msg_q_t *p){
   msg_t* msg;
   if(MsgEmptyQ(p)) {
      cons_printf("Message Queue is empty, can't dequeue!\n");
      return NULL;
   }
   msg = &p->msgs[p->head];
   p->head++;
   if(p->head == NUM_MSG) p->head = 0;
   p->count--;
   return msg;
}



























