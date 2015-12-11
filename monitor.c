#include <time.h> // time()
#include <stdio.h> // perror()
#include "monitor.h" // self
#include "coordination.h"
#define BUFFER_SIZE 128

static unsigned int iteration_count;
static time_t total_time;
/* getters a la Java for the above static variables. */
unsigned int get_iteration_count(){return iteration_count;}
time_t get_total_time(){return total_time;}


static void read_log_file(FILE* log_file)
{
        printf("Reading log.txt: ");

	if (fgetc(log_file) == EOF) {

	        puts("Log empty.");
	        iteration_count = 0;
	        total_time = 0;
	        return;
	}

	fseek(log_file, 1, SEEK_END);
	int ln; for (ln = 1; ln <= 3; ++ln) {

                do fseek(log_file, -2, SEEK_CUR);
                while (fgetc(log_file) != '\n');
        }

	fscanf(log_file, "Iteration %u Total execution time: %u",
        &iteration_count, (unsigned int*) &total_time);

	printf("Previous iterations: %u. Previous time: %u s\n",
	iteration_count, (unsigned int) total_time);

	++iteration_count;
}


static void save_log_file(FILE* log_file, pid_t pid, time_t finish_time)
{       
	if (!log_file) return;
        fprintf(log_file, 
        "Iteration %u\npid: %u Execution time: %u s\nTotal execution time: %u s\n",
        iteration_count, pid, (int) finish_time, (int) total_time);
	fflush(log_file);
}


void* par_wait(void*_)
{
	FILE* log_file = fopen("log.txt", "a+");

        if (!log_file) 
                perror("Couldn't read log.txt. Continuing without logging.");
	else 
	        read_log_file(log_file);
	
	for (;;) {
	        
	        pid_t pid = synced_wait(NULL);
	        regist_wait(pid, time(NULL));
	        time_t finish_time = get_finish_time(pid);
		save_log_file(log_file, pid, finish_time);
		++iteration_count;
		total_time += finish_time;
	}

	fclose(log_file);	 
	return NULL; 		
}
