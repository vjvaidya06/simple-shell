CC = gcc
CFLAGS = -Wall -g

all: mysh

mysh: mysh.c commands.c wildcard.c
	$(CC) $(CFLAGS) -o mysh mysh.c commands.c commands.h wildcard.c wildcard.h

clean:
	rm -f mysh
