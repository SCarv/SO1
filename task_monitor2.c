#include <time.h> // time
#include <stdio.h> // perror
#include <sys/wait.h> // wait
#include <unistd.h> // sleep 
#include <pthread.h> // mutex

#include "task_monitor.h"
#include "list.h"
#include "main.h"

void* task_monitor(void* _main_status)
{

	time_t endtime;
	int pid, waited_children = 0;

	for(;;)
	{
		if (children_count > waited_children) 
		{
	   
			/* In this next part:
			*
			*		1 - We wait for the waited_children-th child to finish
			*		2 - From function wait we get pid
			*		3 - From function time we get the time child took to finish
			*		4 - Save pid, and time into array for later printing 
				FIXME faz update a estes comentarios, que agora ja e com listas e nao com arrays
			*		5 - Check all necessary errors
			*/

			pid = wait(NULL);	// waits here
			endtime = time(NULL);

			if (pid == -1)
			{
				perror("par-shell: [ERROR] couldn't wait on child"); 
				goto failure;
			}
		
			if (endtime < 0)
			{	
				perror("par-shell: [ERROR] couldn't get finish time for child");
			}
			/*FIXME Que valor por na lista se der erro? */
			update_terminated_process(children_list, pid, endtime);
			pthread_mutex_lock(&mutex);

			waited_children += 1;

			pthread_mutex_unlock(&mutex);

		}

		else if (exit_called == 1) //*FIXME por mutex
				goto success;

		else sleep(1); //sleep if no processes are waiting to be waited.

	}

	success: return NULL; //FIXME: este return nao faz sentido
	failure: return NULL; //FIXME: ditto
}
