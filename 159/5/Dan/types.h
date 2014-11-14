// types.h, 159

#ifndef _TYPES_H_
#define _TYPES_H_

#include "trap.h"            // defines tf_t, trap frame type

#define TIME_SLICE 5       // max runtime per run per proc, 2 sec
#define NUM_PROC 20          // max number of processes allowed by OS
#define Q_SIZE 20            // queuing capacity
#define USER_STACK_SIZE 4096 // # of bytes for runtime stack
#define NUM_SEM 20           // number of semaphores by OS
#define NUM_NUM 6            // number of numbers in msg_t
#define NUM_BYTE 101         // number of bytes (chars) in msg_t
#define NUM_MSG 20           // number of msgs in msg_q_t
#define BUFF_SIZE 3          // initial # of msgs for producers

typedef enum {AVAIL, READY, RUN, SLEEP, WAIT} state_t;

typedef void (* func_ptr_t)(); // void-returning function pointer type

typedef struct {          // PCB describes proc image
   state_t state;         // state of process
   int tick_count,        // accumulative run time since dispatched
       total_tick_count,  // accumulative run time since creation
       wake_tick;         // when to wake up when asleep
   tf_t *tf_p;            // process register context
} pcb_t;

typedef struct { // queue type: head & tail pointing array of PIDs
   int count, head, tail, q[Q_SIZE];
} q_t;

typedef struct {
   int sem_count;
   q_t wait_q;
} sem_t;

typedef struct {
   int sender, time_stamp;
   int nums[NUM_NUM];
   char bytes[NUM_BYTE];
} msg_t;

typedef struct {
   int head, tail, count;
   msg_t msgs[NUM_MSG];
} msg_q_t;

typedef struct {
   msg_q_t msg_q;
   q_t wait_q;
} mbox_t;

#endif
