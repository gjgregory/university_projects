/* Garrett Gregory && Josh Richmond, 29 October 2016
 *
 * CSCI 460 Assignment 2
 * ULE Scheduler
 *
 *
 * This .c file implements functions declared in Dispatcher.h that are used by the Simumlate()
 * function in such a way as to create a set of two swapping priority queues as described in
 * the ULE scheduler description. The main() function here simply calls Simulate(), thus starting
 * a simulation of the scheduler.
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
#define MAX_PID 32768
#define MILLION 1000000


struct node {
    int pid;
    struct node *next;
    struct timeval readytime;
	struct timeval ioresponsetime;
};

struct queue {
    struct node *head;
    struct node *tail;
};

// these are inefficient, but won't really affect performance
int aux[MAX_PID]; // array which holds status of pid (waiting(1),running(2),ect)
int was_waiting[MAX_PID];

// two separate queues to switch between
struct queue *current = NULL;
struct queue *next = NULL;

// timing variables
clock_t start_time, end_time;
struct timeval temptimeval;
double readytimes[MAX_PID];
double minreadytime, maxreadytime, avgreadytime, totreadytime, readycount, tempreadytime;
double minresponsetime, maxresponsetime, avgresponsetime, totresponsetime, responsecount, tempresponsetime;


/* Enqueue()
 *
 * Helper function for Ready(). This function creates a process node with the provided pid, adds it
 * to the provided queue, and starts time-keeping for that process.
 */
void enqueue(int pid, struct queue *q){


    struct node *temp = (struct node*)malloc(sizeof(struct node));
    temp->pid = pid;

    // get start time for calculating time in ready queue
	gettimeofday(&temp->readytime, NULL);
	// get start time for calculating i/o response time
	if (was_waiting[pid]){
		temp->ioresponsetime = temp->readytime;
	}

    if(q->head == NULL && q->tail == NULL) {
        q->head = temp;
        q->tail = temp;
        return;
    }
    q->tail->next = temp;
    q->tail = temp;
}

/* Dequeue()
 *
 * Helper function for Dispatch(). This function removes the next process node from the current
 * queue, then performs time calculations.
 */
int Dequeue(struct queue *q) {

    int pid = 0;
    struct node *temp = q->head;
    if (q->head == NULL) {
        if (DEBUG) printf("The queue is empty");
    }
    if (q->head == q->tail) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = q->head->next;
    }
    pid = temp->pid;

    // do ready queue time calculations
    gettimeofday(&temptimeval, NULL);
    readytimes[pid] = (double)((temptimeval.tv_sec*MILLION + temptimeval.tv_usec) -
            (temp->readytime.tv_sec*MILLION +temp->readytime.tv_usec));

    // do i/o response time calculations
    if (was_waiting[pid]){
        gettimeofday(&temptimeval, NULL);
        tempresponsetime = (double)((temptimeval.tv_sec*MILLION + temptimeval.tv_usec) -
                (temp->ioresponsetime.tv_sec*MILLION +temp->ioresponsetime.tv_usec));
        if (!minresponsetime || minresponsetime > tempresponsetime) minresponsetime = tempresponsetime;
        if (maxresponsetime < tempresponsetime) maxresponsetime = tempresponsetime;
        totresponsetime += tempresponsetime;
        responsecount++;
        was_waiting[pid] = 0;
    }

    free(temp);
    return pid;
}

/* NewProcess()
 *
 * This function simply calls Ready() to initialize a new process.
 */
void NewProcess(int pid) {
    start_time = clock();

    aux[pid] = 0; // tells us it is a new process and should be added to the current queue
    if (DEBUG) printf("new process %d created\n", pid);

    end_time += clock() - start_time;

    Ready(pid, 0);
}

/* Dispatch()
 *
 * This function calls Dequeue() to "run" the next process and set *pid to the pid of that process,
 * as well as perform other operations described in Dequeue().
 */
void Dispatch(int *pid) {
    start_time = clock();

    *pid = 0; // zero pid implies an empty ready queue. this should never happen, but ...

    // If the current queue is empty, then the next queue becomes the current queue
    if (current->head == NULL) {
        current->head = next->head;
        current->tail = next->tail;
        next->head = NULL;
        next->tail = NULL;
    }



    // Grab the first element on the current queue for dispatch
    if (current->head != NULL) {
        *pid = Dequeue(current);
        // Set the auxilary data for pid to running
        aux[*pid] = 2;
    }
    if (DEBUG) printf("process %d dispatched\n", *pid);

    end_time += clock() - start_time;
}

/* Ready()
 *
 * This function takes in the pid of a process and performs further operations by calling
 * Enqueue(), which are further described there.
 */
void Ready(int pid, int CPUtimeUsed) {
    // If the process was just created or waited on I/O add it to the current queue
    // If the process did not wait add it to the next queue
    start_time = clock();

    if (aux[pid] == 0) {
        enqueue(pid,current);
    }
    if (aux[pid] == 1) {
        enqueue(pid,current);
    }
    if (aux[pid] == 2) {
        enqueue(pid,next);
    }

    if (DEBUG) printf("process %d added to ready queue after using %d msec\n", pid, CPUtimeUsed);

    end_time += clock() - start_time;
}

/* Waiting()
 *
 * This function indicates that the process with the provided pid is waiting. It sets
 * aux[pid] and was_waiting[pid] to 1, which will be checked by other functions.
 */
void Waiting(int pid) {
    start_time = clock();

    // Set auxilary data to 1 to show pid is waiting
    aux[pid] = 1;
    if (DEBUG) printf("process %d waiting\n", pid);
    was_waiting[pid] = 1;

    end_time += clock() - start_time;
}

/* Terminate()
 *
 * This function indicates that the process with the provided pid was terminated.
 */
void Terminate(int pid) {
    start_time = clock();

    double tempreadytime = readytimes[pid];

    // do ready time calculations
    if (!minreadytime || minreadytime > tempreadytime) minreadytime = tempreadytime;
    if (maxreadytime < tempreadytime) maxreadytime = tempreadytime;
    totreadytime += tempreadytime;
    readycount++;

    if (DEBUG) printf("process %d terminated\n", pid);

    end_time += clock() - start_time;
}

int main() {
    // Initialize the current queue setting pointers to null
    current = (struct queue*)malloc(sizeof(struct queue));
    current->head = NULL;
    current->tail = NULL;

    // Initialize the next queue setting pointers to null
    next = (struct queue*)malloc(sizeof(struct queue));
    next->head = NULL;
    next->tail = NULL;

    // Simulate for 100 rounds, timeslice=100
    Simulate(1000, 100);

    // Free up queues
    free(current);
    free(next);

    // finish time calculations and output results to stdout
	avgreadytime = totreadytime/readycount;
	avgresponsetime = totresponsetime/responsecount;

	printf("\nReady times: min = %.0f, max = %.0f, avg = %.0f (based on real time in usecs)\n", minreadytime, maxreadytime, avgreadytime);
	printf("I/O Response times: min = %.0f, max = %.0f, avg = %.0f (based on real time in usecs)\n", minresponsetime, maxresponsetime, avgresponsetime);
	printf("Total sceduler overhead: %f seconds (based on CPU time)\n\n", (double)end_time);
}
