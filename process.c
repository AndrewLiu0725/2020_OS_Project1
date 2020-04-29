#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "process.h"
#include "scheduler.h"

typedef struct sched_param Sched_pm;
typedef struct timespec Time_sp;

// Define time unit
void inline TIME_UNIT(void){
    volatile unsigned long i;
    for(i = 0; i < 1000000UL; i++);
    return;
}

// Assign the process to specified core
int assign_core(pid_t pid, int core){
    cpu_set_t cpu_mask;

    // Exception check if the core number is invalid
    if (core > sizeof(cpu_mask)){
        fprintf(stderr, "Invalid Core No.: %d\n", core);
        return -1;
    }

    CPU_ZERO(&cpu_mask); // clear cpu_mask 
    CPU_SET(core, &cpu_mask); // set the flag on cpu

    if ( sched_setaffinity(pid, sizeof(cpu_mask), &cpu_mask) != 0 ){
        // Exception check if setaffinity fails
        perror("Ｅrror: sched_setaffinity");
        exit(-1);
    }
    return 0;
}

// The following three functions will be called by each scheduling policy

// Creat and run the child process
pid_t proc_create(Process chld){
    pid_t chpid = fork(); // fork a child process
    
    // Exception check if fork fails
    if ( chpid < 0 ){
        perror("error: fork");
        exit(2);
    }

    // Return 0 means is child process
    if ( chpid == 0 ){
        close( chld.pipe_fd[1] ); // close the pipe write fd for the child
        int init_exec_time = chld.exec_time;
        Time_sp start, end; // Record the start time and end time
        char dmesg[256] = ""; // system message buffer

        // loop if there's time left
        while( chld.exec_time > 0 ){
            char buf[8];
            // Use a blocking system call (read) to synchronize with the scheduler
            read(chld.pipe_fd[0], buf, strlen("run"));
            
            // First time to run
            if( chld.exec_time == init_exec_time ){
                if( clock_gettime(CLOCK_REALTIME, &start) == -1 ){
                    // Exception check if can't get clock time
                    perror("error: clock_gettime");
                    exit(3);
                }
                // Print out child name and its pid (only print once)
                printf("%s %d\n", chld.name, getpid());
            }
            
            TIME_UNIT(); // Run for one time unit
            chld.exec_time--;
        }

        if( clock_gettime(CLOCK_REALTIME, &end) == -1 ){
            // Exception check if can't get clock time
            perror("error: clock_gettime");
            exit(3);
        }

        sprintf(dmesg, "[Project1] %d %09lu.%09lu %09lu.%09lu\n", getpid(), start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);
        syscall(333, dmesg, strlen(dmesg)+1); // Use the self-defined system call to write the time to dmesg
	exit(0);
    }

    // Parent process
    else{
        proc_remove(chpid); // The child process will stop at the read pipe and be set the lowest priority
        assign_core(chpid, CHILD_CORE);
        close(chld.pipe_fd[0]); // close pipe read fd of the parent process, here child is parent process */
        return chpid;
    }
}



// Set prpcess to the lowest prioity possible, i.e. technically remove the process out of the core
int proc_remove(pid_t pid){
    Sched_pm sp;
    sp.sched_priority = 0;

    // Set process to the lowest priority (IDLE)
    if ( sched_setscheduler(pid, SCHED_IDLE, &sp) < 0 ){
        // Exception check if setscheduler fails
        perror("error: sched_setscheduler");
        return -1;
    }
    return 0;
}

// set pid to OTHER priority group, i.e. technically the process is ready to be executed
int proc_exec(pid_t pid){
    Sched_pm sp;
    sp.sched_priority = 0;

    if ( sched_setscheduler(pid, SCHED_OTHER, &sp) < 0 ){
        // Exception check if setscheduler fails
        perror("error: sched_setscheduler");
        return -1;
    }

    return 0;
}
