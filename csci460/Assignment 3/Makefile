CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
LIBRARIES = -lpthread -lrt -lm
OBJECTS_ESC = pagerep_esc.o MemSim.o
OBJECTS_LFU_P1 = pagerep_lfu.o
OBJECTS_LFU_P2 = pagerep_lfu.o MemSim.o
EXES = pagerep_esc pagerep_lfu

pagerep_esc: $(OBJECTS_ESC)
	$(CC) -o pagerep_esc $(LDFLAGS) $(OBJECTS_ESC) $(LIBRARIES)

pagerep_lfu: $(OBJECTS_LFU_P1)
	$(CC) -o pagerep_lfu $(LDFLAGS) $(OBJECTS_LFU_P2) $(LIBRARIES)

all:
	$(MAKE) $(EXES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
