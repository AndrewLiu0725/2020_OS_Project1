#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "scheduler.h"
#include "process.h"


int scheduler_FIFO(Process *proc, int N_procs){
	int proc_index = -1; /* proc_index is the index of the process */
	int total_time=0;

	while(1){
		proc_index += 1; // Start from 0
		if( proc_index >= N_procs ) break; // Break if all processes are finished

		while( proc[proc_index].ready_time > total_time ){
			TIME_UNIT();
			total_time += 1; // wait 1 time unit until proc is ready
		}

		pid_t chpid = proc_create(proc[proc_index]);
		proc_exec( chpid );

        /* keep looping if the execution time is positive */
		while( proc[proc_index].exec_time > 0 ){
			// tell process to run 1 time unit
			write(proc[proc_index].pipe_fd[1], "run", strlen("run"));
			// run 1 time unit itself
			TIME_UNIT();
			total_time += 1;
			proc[proc_index].exec_time -= 1;
		}

		// wait child process
		int re_status;
		waitpid(chpid, &re_status, 0);
		if( !(WIFEXITED(re_status)) ){
			perror("error: child process terminated inappropriately");
			return 1;
		}

	}

	return 0;
}
