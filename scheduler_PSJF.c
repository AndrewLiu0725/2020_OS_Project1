#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include "process.h"

// Basically, this function is the same as the one in scheduler_SJF.c
int preemptive_find_shortest(Process *proc, int N_procs, int time){
	int shortest = -1, excute_time = INT_MAX;
	for (int i = 0; i < N_procs; i++){
		// if the process is ready and its deadline is closer than the current deadline (also check if the execution time is null)
		if ((proc[i].ready_time <= time) && (proc[i].exec_time) && (proc[i].exec_time < excute_time)){
			excute_time = proc[i].exec_time;
			shortest = i;
		}
	}
	return shortest; // return index of the shortest process
}


// Basically, the structure of PSJf is the same as the one of SJF except here it finds the process having shortest deadline in each unit of time
int scheduler_PSJF(Process *proc, int N_procs){
	int time = 0, last_turn = -1;;
	int N_fin = 0;
	int started[N_procs]; // Record whether the process is started or not, 0 means not
	memset(started, 0, sizeof(started));

	
    // Keep looping until all process are finished
	while (N_fin < N_procs){
		int target = preemptive_find_shortest(proc, N_procs, time);
		
        // if such a target exists (initial value is -1)
		if (target != -1){
			// Note that here is preemptive, so every process may be executed more than once
			// Need to check whether the process has been created or not
            // if target has not been created yet
			if (started[target] == 0){
				pid_t chpid = proc_create(proc[target]);
				proc_exec( chpid );
				proc[target].pid = chpid;
				started[target] = 1;
			}
            // target has been created
			else {
				proc_exec( proc[target].pid );
			}
            /* record the index of the process to be run 
               variable 'last_turn' is only used for logging information*/
			last_turn = target;
			write(proc[target].pipe_fd[1], "run", strlen("run"));
			TIME_UNIT();
			time++;
			proc[target].exec_time--;		
			proc_remove( proc[target].pid );
			
            // if the process is finished
			if (proc[target].exec_time == 0){		
				// wait child process
				int _return;
				waitpid(proc[target].pid, &_return, 0);
				
				if (WIFEXITED(_return) != 1){
					fprintf(stderr, "error: child process terminated inappropriately");
					return 1;
				}
				N_fin++;
			}
		}		
        // if there is no such target (all non-finished process are not ready yet)
		else{
			TIME_UNIT();
			time++;
		}
	}
	return 0;
}
