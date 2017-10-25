CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
LIBRARIES = -lpthread -lrt -lm
OBJECTS_A = process_a.o
OBJECTS_B = process_b.o
EXES = process_a process_b

process_a: $(OBJECTS_A)
	$(CC) -o process_a $(LDFLAGS) $(OBJECTS_A) $(LIBRARIES)

process_b: $(OBJECTS_B)
	$(CC) -o process_b $(LDFLAGS) $(OBJECTS_B) $(LIBRARIES)


all:
	$(MAKE) $(EXES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
