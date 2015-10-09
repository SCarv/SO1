# Makefile

par-shell: commandlinereader.o par_run.o main.o
	gcc -o par-shell commandlinereader.o par_run.o main.o

main.o: main.c commandlinereader.h par_run.h
	gcc -Wall -g -c main.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc -Wall -g -c commandlinereader.c

par_run.o: par_run.c commandlinereader.h par_run.h
	gcc -Wall -g -c par_run.c

clean:
	rm -i *.o
