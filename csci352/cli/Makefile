CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS = gg_cli.o gg_calc.o gg_dir.o
EXES = gg_cli

gg_cli:	$(OBJECTS)
	$(CC) -o gg_cli $(LDFLAGS) $(OBJECTS)


all:
	$(MAKE) $(EXES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
