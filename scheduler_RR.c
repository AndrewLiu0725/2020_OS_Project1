#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include "scheduler.h"
#include "process.h"

int scheduler_RR(Process *proc, int N_procs){
	int N_fin = 0; //number of finished processes
	int cur_time = 0; //current time

	while(1){
		int nt = 0; // number of processes not allowed to be executed
		int next_ex_t = INT_MAX; //the closest ready time from current time (used when nt == N_procs)

        // loop through each process
		for(int i = 0; i < N_procs; i++){
			
			// Note that we alrealy sorted the processes according to their ready times

			// The process is not ready yet
			if( cur_time < proc[i].ready_time ){
				if( proc[i].ready_time < next_ex_t ) next_ex_t = proc[i].ready_time; // update the closest ready time if necessary
				nt ++; // increment the number of processes not allowed to be executed since this process cannot be executed right now
				continue;
			}

			// The process is finished
			else if( proc[i].exec_time <= 0 ){
				nt ++; // increment the number of processes not allowed to be executed since this process is finished
				continue;
			} 

            // The process is ready and not finished yet
            // if the process has been created
			if( proc[i].pid > 0 ){
				proc_exec( proc[i].pid );
			}
			// if process hasn't been created (initialized pid is -1)
			else{ 
				proc[i].pid = proc_create( proc[i] );
				proc_exec( proc[i].pid );
			}

			// run an time quantum which is 500 time units in this project
			int kt = RR_SLICE;
			while( proc[i].exec_time > 0 && kt > 0){
				write(proc[i].pipe_fd[1], "run", strlen("run")); // tell process to run 1 time unit
				TIME_UNIT();
				kt --;
				proc[i].exec_time --;
				cur_time ++;
			}

			// if process is finished
			if(proc[i].exec_time <= 0){
				int re_status;
				waitpid(proc[i].pid, &re_status, 0);
				if( !(WIFEXITED(re_status)) ){
					perror("error: child process terminated inappropriately");
					return 1;
				}
				N_fin ++; // increment number of process finished

            // if the process have not yet finished (time slice used up)
			}else{
				proc_remove( proc[i].pid );				
			}

		}

		if( N_fin >= N_procs ) break;

        /* # of processes that can not run >= # of processes */
		if( nt >= N_procs){ 
            // loop until a process is ready
			while( cur_time < next_ex_t ){ 
				TIME_UNIT();
				cur_time ++;
			}
		}
	}
	return 0;
}
