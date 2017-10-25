## File System API

The task was to implement two CPU scheduling algorithms, which are simplified versions of those used in the FreeBSD and the ULE scheduler, then compare their performance with a simulation.

* FileSys.c - Implements functions declared in FileSysAPI.h that are used by the provided tests in order to simulate a file system.

Usage:

```
make clean
make all
./EasyTest
./DriverTest
```
