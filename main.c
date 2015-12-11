#include <stdio.h> // getline()
#include <string.h> // strcmp(), puts()
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "list.h"
#include "par_sync.h"
#include "remotes.h"

#define MSG_PROMPT "Par-shell now ready. Does not wait for jobs to exit!"
#define EMSG_INPUT "\npar-shell: couldn't read input"

#define BUFFER_SIZE 128
#define CHILD_ARGV_SIZE 7
#define EXIT_RETURN -2
#define EXIT_STRING "exit"
#define EXITGLOBAL_RETURN -3
#define EXITGLOBAL_STRING "exit-global"
#define eq_str(str1,str2) (!strcmp(str1,str2)) 

static list_t* children_list;

/* get_child_argv:
Reads all whitespace-separated tokens from the string "command_line"
and saves them in the entries of the 'argv' argument.
This function stops reading when argv arguments are read or a NULL is reached.

Arguments: 
 'argv' should be a vector of char* previously allocated with
 as many entries as 'argv_size'. 
 'command_line' is a string with which will be divided via strtok
 to hold the strings of each argument.
  
 Warning: argv will solely contain pointers to indexes in 'command_line' so 
 'argv's entries are only valid if 'command_line' is in scope.

Return value:
The number of arguments that were read; -1 if any argument evals to false,
because we can't do anything like that.*/
int get_child_argv(char* argv[], size_t argv_size)
{
        char* token;			    // each read token from input. 
        const char delimiters[] =" \t\n";   // strtok-ending characters
        int i, argc;                        // array index; tokens found count
        static char* command_line = NULL;
        static size_t command_size = 0;
        
        FILE* psin = fopen("/tmp/par-shell-in", "r");
        getline(&command_line, &command_size, psin);
        fgetc(psin);
        fclose(psin);
        
        if (!command_line || !argv || !argv_size) return -1;
     
	token = strtok(command_line, delimiters); // this token is the command 

        /* Preencher o vector argv com todos os tokens encontrados
         * ate ultrapassar o tamanho do vector ou chegar a um NULL. */
        for (argc = 0; argc < argv_size-1 && token != NULL; argc++) {
                argv[argc] = token;
                token = strtok(NULL, delimiters);
        }
        
        /* Fill all remaining spaces with null terminators. */
        for (i = argc; i < argv_size; i++) argv[i] = NULL;

        return argc;
}

void par_run(char* argVector[]) 
{
	pid_t pid = synced_fork();
	
	if (!pid) { // Is forked child.

	        char filename[BUFFER_SIZE];
                /* First we redirect the child's output into a file. */
                sprintf(filename, "par-shell-out-%d.txt", getpid());
	        int output_file = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	        dup2(output_file, STDOUT_FILENO);
	        close(output_file);
                /* Then we exec the child. */
                execv(argVector[0], argVector);
                if (argVector[0][0] != '/') 
	                execvp(argVector[0], argVector);
                /* Next line only reached if execv fails. */
                perror("par-shell: exec failed");
                exit(EXIT_FAILURE); // better way of exiting.       
	}
	
	if (pid < 0) perror("par-shell: unable to fork");
	
	else regist_fork(pid, time(NULL));
}

void exitglobal(int signo)
{       
        terminate_remotes();
        threading_cleanup();
        lst_print(children_list);
	lst_destroy(children_list);
	unlink("/tmp/par-shell-in");
	exit(EXIT_SUCCESS);
}

int main() 
{	 
	/* TODO: pipes should be in current working dir. 
	   Use: get_current_dir_name(). But you have to make buffers etc.*/
	char* argv[CHILD_ARGV_SIZE];

        unlink("/tmp/par-shell-in");
        int par_shell_in = mkfifo("/tmp/par-shell-in", S_IRUSR | S_IWUSR);
        if (par_shell_in > 0) perror ("couldn't create FIFO."), exit(1);
        
        signal(SIGINT, exitglobal);
        list_t* children_list = lst_new();
        threading_init(children_list); /** NOTE: initiates multi-threading. */
	
	/* Main loop. The program's main logic is dictated next. */
        for(;;) {
        
                switch (get_child_argv(argv, CHILD_ARGV_SIZE)) {
                        
                        case -1: perror(EMSG_INPUT);
                        case 0: continue;
                        
                        default:
                                if (strtol(argv[0], NULL, 10)) 
                                        acknowledge_remote(argv);
                                        
                                else if (eq_str(argv[0], EXITGLOBAL_STRING)) 
                                        exitglobal(0);
                                        
                                else 
                                        par_run(argv);
                }
        }
}




