#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "commandlinereader.h"
#include "par_run.h"

#define MAX_ARG 7

int main(int argc, char* argv[]) {

	int num_children = 0, status;
	char* argVector[MAX_ARG]; 

	puts("<< PAR-SHELL READY >>");

	for(;;) {

		int numtokens = readLineArguments(argVector, MAX_ARG);

		if (numtokens == -1) {
			perror("par-shell: readLineArguments failed");
			continue; // salta para a proxima interacao se houver erro
		}

		else if (numtokens == 0) {
			puts("(Got nothing)");
			continue;
		}

		else if (!strcmp(argVector[0], "exit")) {

			int i;
			int datav_child[num_children][2]; // vector to save pid and status of each child terminated

			for (i = 0; i < num_children ; i++) {

				if ((datav_child[i][0] = wait(&status)) == -1) // Wait for all processes. If wait fails, break.
				{
					perror("par-shell: wait failed");
					num_children--;
					continue;
				}
				
				datav_child[i][1] = status;
			}

			for (i = 0; i < num_children; i++)
				printf("Process %d finished with status %d.\n", datav_child[i][0], datav_child[i][1]);

			break;
		}

		else if (par_run(argVector) != -1) num_children++;
	}

	return 0;	
}
