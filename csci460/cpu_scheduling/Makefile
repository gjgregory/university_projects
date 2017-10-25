CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
LIBRARIES = -lpthread -lrt -lm
OBJECTS_4BSD = sched_4bsd.o SchedSim.o
OBJECTS_ULE_P1 = sched_ule.o
OBJECTS_ULE_P2 = sched_ule.o SchedSim.o
EXES = sched_4bsd sched_ule

sched_4bsd: $(OBJECTS_4BSD)
	$(CC) -o sched_4bsd $(LDFLAGS) $(OBJECTS_4BSD) $(LIBRARIES)

sched_ule: $(OBJECTS_ULE_P1)
	$(CC) -o sched_ule $(LDFLAGS) $(OBJECTS_ULE_P2) $(LIBRARIES)

all:
	$(MAKE) $(EXES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
