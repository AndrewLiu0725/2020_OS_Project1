#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include "process.h"

int find_shortest(Process *proc, int N_procs, int time){
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


int scheduler_SJF(Process *proc, int N_procs){
	int N_fin = 0;
	int time = 0;

    // Keep looping until all process are finished
	while (N_fin < N_procs){
		int target = find_shortest(proc, N_procs, time);
		
        // if such a target exists (initial value is -1)
		if (target != -1){
			// Note that here is non-preemptive, so every shortest process will be created and executed once only
			// No need to check whether the process has been created or not
			pid_t chpid = proc_create(proc[target]);
			proc_exec( chpid );
			proc[target].pid = chpid;

            // Loop until the process is finished since it's non-preemptive
			while (proc[target].exec_time > 0){
				write(proc[target].pipe_fd[1], "run", strlen("run"));
				TIME_UNIT();
				time++;
				proc[target].exec_time--;
			}
			N_fin++;			

			// Wait child process
			int _return;
			waitpid(proc[target].pid, &_return, 0);
			if (WIFEXITED(_return) != 1){
				fprintf(stderr, "error: child process terminated inappropriately");
				return 1;
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
