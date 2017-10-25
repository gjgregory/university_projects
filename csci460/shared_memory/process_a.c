/* Garrett Gregory, 15 October 2016

 * CSCI 460 Assignment 1
 * Process A
 *
 * This program asks the user for a number and retrieves it from
 * standard input. It then forks a child process and has the child
 * receive the number. The child process then stores the number in
 * shared memory so it can be retrieved by Process B, and waits until
 * Process B gets the number before exiting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

#define MAX_INPUT 64
#define SH_MEM_SIZE 64
#define SEM_NAME_A "garrett_sem1"
#define SEM_NAME_B "garrett_sem2"


int main (int argc, char *argv[]) {
        int pid, i, sh_mem_key, sh_mem_id, semval;
        sem_t *sem_a, *sem_b;
        double *sh_mem;
        unsigned success = 0;
        char *user_input = (char*)malloc(MAX_INPUT);

        // get user input
        printf("May I please have a number? ");
        while (!success){
                fgets(user_input, MAX_INPUT, stdin);
                success = 1;
                // check if input is a single numerical value
                for (i=0; i<(int)strlen(user_input)-1; i++){
                        if (!(isdigit(user_input[i]) || user_input[i] == '.')){
                                success = 0;
                                printf("That wasn't a number... try again please: ");
                                break;
                        }
                }
        }

        // create semaphores
        if ((sem_a = sem_open(SEM_NAME_A, O_CREAT, 0666, 1)) == SEM_FAILED){
                perror("Semaphore creation failed");
                exit(1);
        }
        if ((sem_b = sem_open(SEM_NAME_B, O_CREAT, 0666, 1)) == SEM_FAILED){
                perror("Semaphore creation failed");
                exit(1);
        }

        pid = fork();
        // child process
        if (pid == 0){
                // generate key for shared memory
                if ((sh_mem_key = ftok("/tmp", 'J')) == -1){
                        perror("Shared memory key generation failed.");
                        exit(1);
                }

                // create shared memory segment
                if ((sh_mem_id = shmget(sh_mem_key, SH_MEM_SIZE, IPC_CREAT | 0666)) == -1){
                        perror("Shared memory segment creation failed.");
                        exit(1);
                }

                // attach to shared memory segment
                if (*(sh_mem = (double *) shmat(sh_mem_id, NULL, 0)) == -1) {
                        perror("Attaching to shared memory segment failed.");
                        exit(1);
                }

                // store number in shared memory
                *sh_mem = atof(user_input);

                // post iff process b is waiting
                sem_getvalue(sem_b, &semval);
                if (semval <= 0) sem_post(sem_b);

                printf("Waiting for Process B to retrieve the number...\n");
                sem_wait(sem_a);

                shmdt(&sh_mem);
                exit(0);
        }
        // parent process
        else{
                int retStatus;
                waitpid(pid, &retStatus, 0);

                sem_close(sem_a);
                sem_close(sem_b);
        }
}
