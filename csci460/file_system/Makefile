CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
LIBRARIES = -lpthread -lrt -lm
OBJECTS_REQ = FileSys.o Driver.o
OBJECTS_EASY = EasyTest.o
OBJECTS_API = APITest.o
OBJECTS_DRIVER = DriverTest.o
EXES = EasyTest APITest DriverTest

EasyTest: $(OBJECTS_EASY) $(OBJECTS_REQ)
	$(CC) -o EasyTest $(LDFLAGS) $(OBJECTS_EASY) $(OBJECTS_REQ) $(LIBRARIES)

APITest: $(OBJECTS_API) $(OBJECTS_REQ)
	$(CC) -o APITest $(LDFLAGS) $(OBJECTS_API) $(OBJECTS_REQ) $(LIBRARIES)

DriverTest: $(OBJECTS_DRIVER) $(OBJECTS_REQ)
	$(CC) -o DriverTest $(LDFLAGS) $(OBJECTS_DRIVER) $(OBJECTS_REQ) $(LIBRARIES)

all:
	$(MAKE) $(EXES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
