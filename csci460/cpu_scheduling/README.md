## CPU Scheduling

The task was to implement two CPU scheduling algorithms, which are simplified versions of those used in the FreeBSD and the ULE scheduler, then compare their performance with a simulation.

* sched_4bsd.c - Simplified version of FreeBSD.

* sched_ule - Simplified version of ULE scheduler.

Usage:

```
make clean
make all
./sched_4bsd
./sched_ule
```
