#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "par_run.h"


int par_run(char* argVector[]) {

	int pid = fork();

	if (pid == -1)  //parent. if child isn't created
		perror("par-shell: unable to fork");		

	else if (pid == 0) { // child
		execv(argVector[0], argVector);
		// next line shouldn't be reached
		perror("par-shell: exec failed");
		exit(1);
	}

	return pid; // parent
}
