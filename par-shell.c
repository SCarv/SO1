#include <stdio.h> // getline()
#include <string.h> // strcmp(), puts()
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "list.h"
#include "coordination.h"
#include "remotes.h"

#define MSG_PROMPT "Par-shell now ready. Does not wait for jobs to exit!"
#define EMSG_INPUT "\npar-shell: couldn't read input"

#define BUFFER_SIZE 128
#define CHILD_ARGV_SIZE 7
#define PAR_SHELL_IN_PATH "in"

static list_t* children_list;

char* read_input_pipe()
{
        static char* command_line = NULL;
        static size_t command_size = 0;

	puts("Now listening on input pipe!");
        FILE* psin = fopen(PAR_SHELL_IN_PATH, "r");
        getline(&command_line, &command_size, psin);
        fgetc(psin); // remove null terminator from the buffer
        fclose(psin);
        puts("Got command line from input pipe!");

	return command_line;
}

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
        int i, argc;                        // array index; tokens found count

        const char delimiters[] =" \t\n";   // sttok-ending characters
	char* command_line = read_input_pipe();

        if (!argv || !argv_size) return -1;

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

void exec(char* argv[])
{
	char filename[BUFFER_SIZE];
        /* First we redirect the child's output into a file. */
        sprintf(filename, "par-shell-out-%d.txt", getpid());
	int output_file = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	dup2(output_file, STDOUT_FILENO);
	close(output_file);

        /* Then we exec the child. */
        execv(argv[0], argv);
        if (argv[0][0] != '/') execvp(argv[0], argv);

        /* Next line only reached if exec fails. */
        perror("par-shell: exec failed");
	_exit(EXIT_FAILURE); // child exits
}

void run(char* argv[])
{
	pid_t pid = synced_fork();

	if (!pid) exec(argv); // Is forked child

	printf("Running: ");
	int i = 0;
	char* arg = argv[0];
	while (arg) printf("%s ", arg), arg = argv[++i];
	putchar('\n');

	if (pid < 0) perror("par-shell: couldn't fork");
	else regist_fork(pid, time(NULL));
}

void exit_global(int signo)
{
        puts("Caught exit-global or SIGINT. Terminating all remotes and exiting.");
        terminate_remotes();
        threading_cleanup();
        lst_print(children_list);
	lst_destroy(children_list);
	unlink(PAR_SHELL_IN_PATH);
	exit(EXIT_SUCCESS);
}

int main()
{
	/* TODO: pipes should be in current working dir.
	   Use: get_current_dir_name(). But you have to make buffers etc.*/
	char* argv[CHILD_ARGV_SIZE];
        unlink(PAR_SHELL_IN_PATH);
        children_list = lst_new();
        threading_init(children_list);
        mkfifo(PAR_SHELL_IN_PATH, S_IRUSR | S_IWUSR) >= 0
        ?1: (perror ("couldn't create FIFO."), exit(1));
        signal(SIGINT, exit_global);
	/* Main loop. The program's par-shell logic is dictated next.*/
        for(;;) {

                switch(get_child_argv(argv, CHILD_ARGV_SIZE)) {

                        case 0: puts("Received empty message."); break;
                        case -1: perror(EMSG_INPUT); break;
                        default:
                        if (!strcmp(argv[1], "exit-global")) exit_global(0);
                        else if (!strcmp(argv[1], "stats")) stats(strtoul(argv[1], NULL, 10));
                        else if (!strcmp(argv[1], "regist")) acknowledge(argv);
                        else run(argv);
                }
        }
}

