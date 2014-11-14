// q_mgmt.c, 159

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "pq_mgmt.h"


int EmptyPQ(pq_t *p)
{
   return (p->count == 0);
}

int FullPQ(pq_t *p)
{
	return (p->count == Q_SIZE+1);
}

void InitPQ(pq_t *p)
{
	p->count = 0;
}


void EnPQ(int pid, int sleep, pq_t *p)
{
	int hole;
	int element[2];
	
	element[0] = pid;
	element[1] = sleep;
    
	if(FullPQ(p)){
        cons_printf("Full Queue Error\n");
        return;
    }
   
    for(hole = ++p->count; 
        hole > 1 && element[WAIT_TIME] < p->pq[(hole + 1)/2][WAIT_TIME];
        hole = (hole + 1)/2) 
    {
        p->pq[hole][PID] = p->pq[(hole + 1)/2][PID];
        p->pq[hole][WAIT_TIME] = p->pq[(hole + 1)/2][WAIT_TIME];
    }
    p->pq[hole][PID] = element[PID];
    p->pq[hole][WAIT_TIME] = element[WAIT_TIME];
}


int DePQ(pq_t *p) // return -1 if q is empty
{
    int pid, newPos;
    
	if(EmptyPQ(p)){
        cons_printf("Empty Queue Error\n");
        return -1;
    }
    p->count--;
    pid = p->pq[1][PID];
    
    newPos = PercolateDown(p, 1, p->pq[p->count + 1][WAIT_TIME]);
    p->pq[newPos][PID] = p->pq[p->count + 1][PID];
    p->pq[newPos][WAIT_TIME] = p->pq[p->count + 1][WAIT_TIME];
    
	return pid;
}


int PercolateDown(pq_t *p, int hole, int priority)
{
    int left, right, target;;
    
    
    while(hole * 2 <= p->count) {
        left = hole * 2;
        right = left + 1;
        if(right <= p->count && p->pq[right][WAIT_TIME] < p->pq[left][WAIT_TIME])
        {
            target = right;
        }
        else {
            target = left;
        }
        if(p->pq[target][WAIT_TIME] < priority){
            p->pq[hole][PID] = p->pq[target][PID];
            p->pq[hole][WAIT_TIME] = p->pq[target][WAIT_TIME];
            hole = target;
        }
        else break;        
    }
    return hole;
}

void printHeap(pq_t *p)
{
    int i = 1;
    if(p->count == 0){
        cons_printf("Empty\n");
        return;
    }
    for(; i <= p->count; i++) {
        cons_printf("[%d, %d]  ", p->pq[i][PID], p->pq[i][WAIT_TIME]);
    }
    cons_printf("\n");
}

void printTop(pq_t *p){
    cons_printf("[%d, %d]\n", p->pq[1][PID], p->pq[1][WAIT_TIME]);
}

int peek(pq_t *p){
	return p->pq[1][WAIT_TIME];
}

/*
int main() {
    pq_t test;
    int data[10] = {2, 7, 2, 5, 0, 9, 3, 1, 7, 8};
    int i = 0;
    int element[2];
    InitPQ(&test);
    printHeap(&test);
    for(; i < 10; i++) {
        element[PID] = i;
        element[WAIT_TIME] = data[i];
        EnPQ(element, &test);
    }
    
    printHeap(&test);
    
    while(test.count) {
        printTop(&test);
        DePQ(&test);        
    }
    
    printHeap(&test);
    
    for(i = 0; i <= Q_SIZE; i++) {
        element[PID] = i + 1;
        element[WAIT_TIME] = 2147483647 - i - (i%2);
        EnPQ(element, &test);
    }
    printHeap(&test);
    while(test.count) {
        printTop(&test);
        DePQ(&test);        
    }
    
    for(i=1; i<Q_SIZE; i++){
        element[PID] = i;
        element[WAIT_TIME] = ((Q_SIZE + i)*(Q_SIZE + i + 1))%19;
        EnPQ(element, &test);
    }
    printHeap(&test);
    do {
        printTop(&test);
        DePQ(&test);
    } while(test.count);
    return 0;
}
*/
