#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "scheduler.h"
#include "process.h"

// This function is used for qsort the processes by thier ready time (or name if they have same ready time)
int CompareProcessSort(const void *pnt_a, const void *pnt_b){
	Process *proc_a = (Process *)pnt_a;
	Process *proc_b = (Process *)pnt_b;
	
	if( proc_a->ready_time < proc_b->ready_time){
		return -1;
	}
	else if( proc_a->ready_time > proc_b->ready_time){
		return 1;
	}
	else{
		return strcmp(proc_a->name, proc_b->name);
	}
}

int main(){
	// Read inputs from test data
    // Scheduling policy FIFO, RR, SJF, and PSFJ
	char SP[8];
	scanf("%s", SP);

    // # of processes
	int n;
	scanf("%d", &n);

    // Allocate memory for n processes
	Process* proc = malloc(n * sizeof(Process));

    // Read the details of each process
	for(int i = 0 ; i < n ; i++){
		scanf("%s%d%d", (proc + i) -> name, &((proc + i) -> ready_time), &((proc + i) -> exec_time));

        // Initialize pid to -1
		(proc + i) -> pid = -1; 

        // create a pipe between pipe_fd
		if(pipe(((proc + i) -> pipe_fd)) < 0){
			perror("error: pipe");
			exit(1);
		}
	}

    // Sort the processes in ascending order according to their ready time (or their indices if their ready time are the same)
	qsort(proc, n, sizeof(Process), CompareProcessSort);

    // Assign the main process to cpu 0, assign_core is defined in process.c, SCHED_CORE is defined in scheduler.h
	assign_core(getpid(), SCHED_CORE);
	
    // Schedule the emulated processes
	if(strcmp(SP, "FIFO") == 0){
		scheduler_FIFO(proc, n);
	}
	else if(strcmp(SP, "RR") == 0){
		scheduler_RR(proc, n);
	}
	else if(strcmp(SP, "SJF") == 0){
		scheduler_SJF(proc, n);
	}
	else if(strcmp(SP, "PSJF") == 0){
		scheduler_PSJF(proc, n);
	}
	else{
		printf("No Such scheduler policy.\n");
	}

	return 0;
}
