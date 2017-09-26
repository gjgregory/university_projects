CC = gcc
CFLAGS = -g
POBJECT = prog3_participant.o
OOBJECT = prog3_observer.o
SOBJECT = prog3_server.o
EXES = prog3_participant prog3_observer prog3_server

all: prog3_participant prog3_observer prog3_server

prog3_participant:	$(POBJECT)
	$(CC) -o $@ $(LDFLAGS) $(POBJECT)

prog3_observer: 	$(OOBJECT)
	$(CC) -o $@ $(LDFLAGS) $(OOBJECT)

prog3_server: $(SOBJECT)
	$(CC) -o $@ $(LDFLAGS) $(SOBJECT)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
