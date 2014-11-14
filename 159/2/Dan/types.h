// types.h, 159

#ifndef _TYPES_H_
#define _TYPES_H_

#include "trap.h"            // defines tf_t, trap frame type
#include "spede.h"

#define TIME_SLICE 200       // max runtime per run per proc, 2 secs
#define NUM_PROC 20          // max number of processes allowed in the system
#define Q_SIZE 20            // queuing capacity
#define USER_STACK_SIZE 4096 // # of bytes for runtime stack

typedef enum {AVAIL, READY, RUN, SLEEP, WAIT} state_t;

typedef struct            // PCB describes proc image
{
   state_t state;         // state of process
   int tick_count,        // accumulative run time since dispatched
       total_tick_count;  // accumulative run time since creation

   tf_t *tf_p;            // process register context
} pcb_t;

typedef struct // queue type: head & tail pointing array of PIDs
{
   int count, head, tail, q[Q_SIZE];
} q_t;

typedef enum {PID, WAIT_TIME} waitQ_t;

typedef struct // Priority queue
{
    int count, pq[Q_SIZE+1][2];
} pq_t;

typedef void (* func_ptr_t)(); // void-returning function pointer type

#endif
