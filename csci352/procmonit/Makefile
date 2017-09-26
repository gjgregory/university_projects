CFLAGS = -Wall -g 
CC = gcc


all : procmonit

% : %.c
	$(CC) $(CFLAGS) -o $@ $< `pkg-config --cflags --libs gtk+-3.0`
