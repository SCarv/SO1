# Makefile for par-shell.

CFLAGS=-pthread -Wall -Werror -g
# Flags detailed:
# pthread: enable multithreading
# Wall: All warnings.
# Werror: Warnings become errors.
# g: enable debugging symbols

all: par-shell par-shell-terminal

par-shell-terminal: par-shell-terminal.c 
	$(CC) $(CFLAGS) -o par-shell-terminal par-shell-terminal.c

par-shell: list.o remotes.o coordination.o monitor.o par-shell.o 
	$(CC) $(CFLAGS) -o par-shell list.o remotes.o coordination.o monitor.o par-shell.o

par-shell.o: par-shell.c coordination.h list.h remotes.h
	$(CC) $(CFLAGS) -c -o par-shell.o par-shell.c

coordination.o: coordination.c monitor.h coordination.h list.h
	$(CC) $(CFLAGS) -c -o coordination.o coordination.c

monitor.o: monitor.c monitor.h coordination.h
	$(CC) $(CFLAGS) -c -o monitor.o monitor.c

remotes.o: remotes.c remotes.h monitor.h
	$(CC) $(CFLAGS) -c -o remotes.o remotes.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

clean:
	rm *.o *.txt

fibonacci: fibonacci.c
	$(CC) fibonacci.c -o fibonacci

remove: 
	rm par-shell fibonacci

test: par-test.sh fibonacci par-shell
	./par-test.sh

delete:
	rm fibonacci par-shell par-shell-terminal
