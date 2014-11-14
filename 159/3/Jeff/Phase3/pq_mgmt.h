#ifndef _PQ_MGMT_H_
#define _PQ_MGMT_H_


int EmptyPQ(pq_t *);
int FullPQ(pq_t *);
void InitPQ(pq_t *);
int DePQ(pq_t *);
void EnPQ(int, int, pq_t *);
int PercolateDown(pq_t *, int, int);
void printHeap(pq_t *);
int peek(pq_t *);


#endif
