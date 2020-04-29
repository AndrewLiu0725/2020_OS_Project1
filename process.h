#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sys/types.h>

// Define a structure called 'Process'

typedef struct{
    char name[32];
    int ready_time; 
    int exec_time;
    pid_t pid;
    int pipe_fd[2]; // two file descriptors used by pipes
}Process;


void inline TIME_UNIT(void);

// core: 0 for scheduler, 1 for child processes
int assign_core(pid_t pid, int core);

// spawn new process when ready_time is reached
pid_t proc_create(Process chld);

// remove child out of core
int proc_remove(pid_t pid);

// bring another child to run
int proc_exec(pid_t pid);

#endif
