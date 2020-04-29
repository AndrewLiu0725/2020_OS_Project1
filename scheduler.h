#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "process.h"
// maximum time units in Round-Robin policy
#define RR_SLICE 500

// Assign different core to scheduler/processes
#define SCHED_CORE 0
#define CHILD_CORE 1

/* 
Given to scheduler:
   1. details of each process
   2. number of processes
*/

int scheduler_FIFO(Process *proc, int N_procs);
int scheduler_RR(Process *proc, int N_procs);
int scheduler_SJF(Process *proc, int N_procs);
int scheduler_PSJF(Process *proc, int N_procs);

#endif