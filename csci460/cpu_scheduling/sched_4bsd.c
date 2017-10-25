/* Garrett Gregory && Josh Richmond, 29 October 2016
 *
 * CSCI 460 Assignment 2
 * 4BSD Scheduler
 *
 *
 * This .c file implements functions declared in Dispatcher.h that are used by the Simumlate()
 * function in such a way as to create a 16-level priority queue. The main() function here simply
 * calls Simulate(), thus starting a simulation of the scheduler.
 *
 *
 * ***NOTE: THE TIMERS WILL NOT WORK PROPERLY IF THIS IS RUN JUST BEFORE MIDNIGHT***
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "Dispatcher.h"
#include "SchedSim.h"

#define DEBUG 0
#define MAX_PID 32768 // this is equal to max_pid
#define DEFAULT_PRIORITY 8
#define MAX_PRIORITY 16
#define MILLION 1000000


struct node {
  	int pid;
  	struct node *next;
  	struct node *prev;
	struct timeval readytime;
	struct timeval ioresponsetime;
};

struct queue {
  	struct node *head;
  	struct node *tail;
};

// 16 ready queues for their corresponding priority level
struct queue *ready_queue[MAX_PRIORITY];

// these are inefficient, but won't really affect performance
int prior_vals[MAX_PID]; // maintains priorities for each process
int was_waiting[MAX_PID];

// timing variables
clock_t start_time, end_time;
struct timeval temptimeval;
double readytimes[MAX_PID];
double minreadytime, maxreadytime, avgreadytime, totreadytime, readycount, tempreadytime;
double minresponsetime, maxresponsetime, avgresponsetime, totresponsetime, responsecount, tempresponsetime;


/* NewProcess()
 *
 * This function simply calls Ready() to initialize a new process.
 */
void NewProcess(int pid) {
    start_time = clock();
	if (DEBUG) printf("new process %d created\n", pid);
    end_time += clock() - start_time;
	Ready(pid, 0);
}

/* Dispatch()
 *
 * This function finds the first process node available, starting from the highest-priority
 * queue and ending at the lowest-priority queue. It sets the *pid parameter to that node's pid,
 * does time calculations, and removes the process node from its ready queue.
 */
void Dispatch(int *pid) {
	int i = 0;
	*pid = 0; // zero pid implies an empty ready queue. this should never happen, but ...
	struct node *tempnode;

    start_time = clock();

	// select node from first (highest priority) queue that isn't empty
	while (i < MAX_PRIORITY){
		tempnode = ready_queue[i]->head;
		// if not empty...
		if (ready_queue[i]->head != NULL){
			*pid = ready_queue[i]->head->pid;
			// if more than one node in queue, set queue head to next node in queue
			if (ready_queue[i]->head->next->pid > 0){
				ready_queue[i]->head = ready_queue[i]->head->next;
				ready_queue[i]->head->prev = NULL;
			}
			// if more than one node in queue, set queue head to NULL
			else ready_queue[i]->head = NULL;

			// do ready queue time calculations
			gettimeofday(&temptimeval, NULL);
			readytimes[*pid] += (double)((temptimeval.tv_sec*MILLION + temptimeval.tv_usec) -
					(tempnode->readytime.tv_sec*MILLION +tempnode->readytime.tv_usec));

			// do i/o response time calculations
			if (was_waiting[*pid]){
				gettimeofday(&temptimeval, NULL);
				tempresponsetime = (double)((temptimeval.tv_sec*MILLION + temptimeval.tv_usec) -
						(tempnode->ioresponsetime.tv_sec*MILLION +tempnode->ioresponsetime.tv_usec));
				if (!minresponsetime || minresponsetime > tempresponsetime) minresponsetime = tempresponsetime;
				if (maxresponsetime < tempresponsetime) maxresponsetime = tempresponsetime;
				totresponsetime += tempresponsetime;
				responsecount++;
				was_waiting[*pid] = 0;
			}

			free(tempnode);
			if (DEBUG) printf("process %d dispatched\n", *pid);
			break;
		}
		i++;
	}
    end_time += clock() - start_time;
}

/* Ready()
 *
 * This function takes in the pid of a process, creates a node for that process, and adds that node
 * to a ready queue indexed by its priority minus 1 (or priority = 8 minus 1 as default). It then starts
 * timers for ready, and response if the process was previously waiting.
 */
void Ready(int pid, int CPUtimeUsed) {
	struct node *new_node;

    start_time = clock();

	if (prior_vals[pid] <= -1) prior_vals[pid] = DEFAULT_PRIORITY; // set default priority if new process
	else if (prior_vals[pid] < 16) prior_vals[pid]++; // this will be negated if process waited.

	// construct new process node
	new_node = (struct node*)malloc(sizeof(struct node));
	new_node->pid = pid;

	// add new process node to tail of ready queue
	if (ready_queue[prior_vals[pid]-1]->head == NULL){ 			// ready queue is empty
		ready_queue[prior_vals[pid]-1]->head = new_node;
		ready_queue[prior_vals[pid]-1]->head->next = new_node;
		new_node->next = ready_queue[prior_vals[pid]-1]->tail;
		ready_queue[prior_vals[pid]-1]->tail->prev = new_node;
	}
	else { 									//ready queue is not empty
		ready_queue[prior_vals[pid]-1]->tail->prev->next = new_node;
		new_node->prev = ready_queue[prior_vals[pid]-1]->tail->prev;
		new_node->next = ready_queue[prior_vals[pid]-1]->tail;
		ready_queue[prior_vals[pid]-1]->tail->prev = new_node;
	}
	if (DEBUG) printf("process %d added to ready queue after using %d msec (priority = %d)\n", pid, CPUtimeUsed, prior_vals[pid]);

	// get start time for calculating time in ready queue
	gettimeofday(&new_node->readytime, NULL);
	// get start time for calculating i/o response time
	if (was_waiting[pid]){
		new_node->ioresponsetime = new_node->readytime;
	}
    end_time += clock() - start_time;
}

/* Waiting()
 *
 * This function indicates that the process with the provided pid is waiting. It subtracts from
 * the process's priority by 2 (will be incremented by 1 in ready) and sets was_waiting[pid] equal
 * to 1 for time-keeping purposes.
 */
void Waiting(int pid) {
    start_time = clock();

	if (prior_vals[pid] > 1) prior_vals[pid] -= 2;
	else prior_vals[pid]--;

	if (DEBUG) printf("process %d waiting\n", pid);
	was_waiting[pid] = 1;

    end_time += clock() - start_time;
}


/* Terminate()
 *
 * This function indicates that the process with the provided pid was terminated. It does some time
 * calculations for ready queue times and resets the priority of the process with provided pid.
 */
void Terminate(int pid) {
    start_time = clock();

    double tempreadytime = readytimes[pid];
	prior_vals[pid] = 0;

    // do ready time calculations
    if (!minreadytime || minreadytime > tempreadytime) minreadytime = tempreadytime;
    if (maxreadytime < tempreadytime) maxreadytime = tempreadytime;
    totreadytime += tempreadytime;
    readycount++;

	if (DEBUG) printf("process %d terminated\n", pid);

    end_time += clock() - start_time;
}

int main() {
	// initialize all priorities to -1 (unset)
	int i;
	for (i=0; i < MAX_PID; i++){
		prior_vals[i] = -1;
	}

	// initialize ready queues
	for (i=0; i<MAX_PRIORITY; i++){
		ready_queue[i] = (struct queue*)malloc(sizeof(struct queue));
	        ready_queue[i]->head = NULL;
	        ready_queue[i]->tail = (struct node*)malloc(sizeof(struct node));;
	}

	// simulate for 100 rounds, timeslice=100
	Simulate(1000, 100);

	// free up ready queues
	for (i=0; i<MAX_PRIORITY; i++){
		free(ready_queue[i]->tail);
		free(ready_queue[i]);
	}

	// finish time calculations and output results to stdout
	avgreadytime = totreadytime/readycount;
	avgresponsetime = totresponsetime/responsecount;
	printf("\nReady times: min = %.0f, max = %.0f, avg = %.0f (based on real time in usecs)\n", minreadytime, maxreadytime, avgreadytime);
	printf("I/O Response times: min = %.0f, max = %.0f, avg = %.0f (based on real time in usecs)\n", minresponsetime, maxresponsetime, avgresponsetime);
	printf("Total sceduler overhead: %f seconds (based on CPU time)\n\n", (double)end_time);
}
