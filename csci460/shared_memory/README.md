## Shared Memory

The task was to implement information storing between two processes through shared memory. I also got a bit carried away and added synchronization to the project description.

* process_a.c - This program asks the user for a number and retrieves it from standard input. It then forks a child process and has the child receive the number. The child process then stores the number in shared memory so it can be retrieved by Process B, and waits until Process B gets the number before exiting.

* process_b.c - This program first forks a child process. Then, if Process A has stored a number in shared memory and is waiting for Process B (or has exited), it will retrieve the number from shared memory, mod the number by 26, then pipe the new number to the parent process. The parent process will then display that number on standard output.

Usage (Synchronized, so use two separate terminal windows):

```
make clean
make all
./process_a
./process_b
```
