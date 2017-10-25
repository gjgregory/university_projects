/* Garrett Gregory, 15 October 2016

 * CSCI 460 Assignment 1
 * Process B
 *
 * This program first forks a child process. Then, if Process A has stored
 * a number in shared memory and is waiting for Process B (or has exited),
 * it will retrieve the number from shared memory, mod the number by 26, then
 * pipe the new number to the parent process. The parent process will then
 * display that number on standard output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <math.h>

#define MAX_INPUT 64
#define SH_MEM_SIZE 64
#define MOD_BY 26.0
#define SEM_NAME_A "garrett_sem1"
#define SEM_NAME_B "garrett_sem2"


int main (int argc, char *argv[]) {
        int pid, sh_mem_key, sh_mem_id, pipe_fd[2], semval;
        sem_t *sem_a, *sem_b;
        double *sh_mem, number;

        // create simple pipe
        pipe(pipe_fd);

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
                // close input end of pipe
                close(pipe_fd[0]);

                // wait for process a to store a number in shared memory
                printf("Waiting on Process A...\n");
                sem_wait(sem_b);

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

                // get shared data, calculate new number and send through pipe
                number = fmod(*sh_mem, MOD_BY);
                shmdt(&sh_mem);
                write(pipe_fd[1], &number, sizeof(number));

                // post iff process a is waiting
                sem_getvalue(sem_a, &semval);
                if (semval <= 0) sem_post(sem_a);

                exit(0);
        }
        // parent process
        else{
                // close output end of pipe
                close(pipe_fd[1]);

                int retStatus;
                waitpid(pid, &retStatus, 0);

                read(pipe_fd[0], &number, sizeof(number));
                printf("\nThe (rounded) number you were looking for is: %.2f\n\n", number);

                sem_close(sem_a);
                sem_close(sem_b);
        }
}
