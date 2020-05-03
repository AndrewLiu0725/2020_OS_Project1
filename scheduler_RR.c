#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include "scheduler.h"
#include "process.h"

// Defien operations about queue
#define CAPACITY 20
Process** queue; 
unsigned int size  = 0;
unsigned int rear  = CAPACITY - 1;   // Initally assumed that rear is at end
unsigned int front = 0;

int enqueue(Process*);
Process* dequeue();
int isFull();
int isEmpty();

int enqueue(Process* proc){
    // Queue is full throw Queue out of capacity error.
    if (isFull()) {
		printf("Error: queue is full!");
        return 0;
    }
    // Ensure rear never crosses array bounds
    rear = (rear + 1) % CAPACITY;
    // Increment queue size
    size++;
    // Enqueue new element to queue
    queue[rear] = proc;
    // Successfully enqueued element to queue
    return 1;
}

Process* dequeue(){
    // Queue is empty, throw Queue underflow error
    if (isEmpty()){
        return NULL;
    }

    // Dequeue element from queue
    Process* proc = queue[front];

    // Ensure front never crosses array bounds
    front = (front + 1) % CAPACITY;

    // Decrease queue size
    size--;

    return proc;
}

int isFull(){
    return (size == CAPACITY);
}

int isEmpty(){
    return (size == 0);
}

int scheduler_RR(Process *proc, int N_procs){
	queue = malloc(CAPACITY * sizeof(Process*)); // Initialize the queue
	int N_fin = 0; // number of finished processes
	int cur_t = 0; // current time
	int rh = 0;    // ready head
 	Process* last_process = NULL; // In the first place, there is no unfinished process

	while(1){
		// Maintain the waiting queue after each iteration
		// Enqueue the process which were ready in the last iteration
		while(cur_t >= proc[rh].ready_time && rh < N_procs){
			enqueue(&proc[rh]);
			rh++;
		}
		// Enqueue last run unfinished process
		if(last_process != NULL) enqueue(last_process);


		// Run the first in process for 1 time quantum if there is process in the waiting queue
		// if there is no waiting process, jump to the next if block
		if(!isEmpty()){
			Process* process = dequeue();
			if( process -> pid > 0 ){
				proc_exec( process -> pid );
			}else{
				process -> pid = proc_create(*process);
				proc_exec( process -> pid );
			}

			int kt = 500; //time quantum for RR
			// Run for one time quantum
			while( process -> exec_time > 0 && kt > 0){
				write(process -> pipe_fd[1], "run", strlen("run")); // tell process to run 1 time unit
				TIME_UNIT(); // run 1 time unit itself
                /* decrement time slice, process execution time 
                   and increment current time */
				kt --;
				process -> exec_time --;
				cur_t ++;
			}

			// if the process is finished, receive its exit status
			if(process -> exec_time <= 0){
				int re_status;
				waitpid(process -> pid, &re_status, 0);
				if( !(WIFEXITED(re_status)) ){
					perror("error: child process terminated inappropriately");
					return 1;
				}
                /* increment number of process finished */
				N_fin ++;
				last_process = NULL;
			}
			
			// if the process has not yet finished (time slice used up) 
			else{
				proc_remove( process -> pid );	
				last_process = process;			
			}
		}

		// There is no process in waiting queue && there is still unready process && the previous process is finished
		if (isEmpty() && (rh < N_procs) && (last_process == NULL)){
			// this loop will run if there is no process ready in the last time quantum
			while(cur_t < proc[rh].ready_time){
				// loop until a process is ready, i.e. until cur_t = proc[rh].ready_time
				TIME_UNIT();
				cur_t ++;
			}
		}

		// return when all processes are finished
		if(N_fin >= N_procs){
			break;
		}
	}
	return 0;
}