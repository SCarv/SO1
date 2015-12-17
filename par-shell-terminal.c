#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#define BUFFER_SIZE 128
#define CHILD_ARGV_SIZE 7
#define EMSG_INPUT "\npar-shell: couldn't read input"
#define EMSG_PIPE "\npar-shell: couldn't open pipe"
#define MSG_IN "\nPath for par-shell-in pipe (existing): "
#define MSG_OUT "\nPath for par-shell-out pipe (will be created): "


void input_error()
{
        perror(EMSG_INPUT);
        exit(EXIT_FAILURE);
}

void pipe_error()
{
        perror(EMSG_PIPE);
        exit(EXIT_FAILURE);
}

void regist_self(char* par_shell_in, char* par_shell_out)
{
        FILE* psin = fopen(par_shell_in, "w");
        if (!psin) pipe_error();
        fprintf(psin, "%u regist %s", getpid(), par_shell_out);
        fclose(psin);
}

void get_stats(char* par_shell_in, char* par_shell_out, char** buffer, size_t* buffer_size)
{   
        regist_self(par_shell_in, par_shell_out);
        
        FILE* psout = fopen(par_shell_out, "r");
        
        getline(buffer, buffer_size, psout); // get number of iters
        printf("\nIterations: %s", *buffer);
        fgetc(psout); // remove null
        
        getline(buffer, buffer_size, psout);
        printf("\nTotal time: %s", *buffer);
        fgetc(psout); // remove null
        
        fclose(psout);
}   

void command(const char command_line[], const char par_shell_in_path[])
{
        char pid_command_line[BUFFER_SIZE];
        sprintf(pid_command_line, "%u %s", getpid(), command_line);
        FILE* psin = fopen(par_shell_in_path, "w");
        if (psin) fputs(pid_command_line, psin);
        else pipe_error();
        fclose(psin);
}

int main()
{
        char* input = NULL; // for getline
        size_t size = 0; // for getline
        size_t input_len;
        
        printf(MSG_IN);
        if (getline(&input, &size, stdin) < 0) input_error();
        strtok(input, "\n"); //strip newline
        char* par_shell_in_path = strdup(input);
        
        printf(MSG_OUT);
        if (getline(&input, &size, stdin) < 0) input_error();
        strtok(input, "\n"); //strip newline
        char* par_shell_out_path = strdup(input);
        
        unlink(par_shell_out_path);
        if (mkfifo(par_shell_out_path, S_IRUSR | S_IWUSR) < 0) pipe_error();
        
        regist_self(par_shell_in_path, par_shell_out_path);
        
	for(;;) {
	
                printf("\n>>> ");
                input_len = getline(&input, &size, stdin);
                
                if (input_len < 0) perror(EMSG_INPUT);
                
                else if (input_len > 0) {
                
                        if (!strncmp("exit", input, 4)) break;

                        else if (!strncmp("stats", input, 5))
                        	get_stats(par_shell_in_path, par_shell_out_path, &input, &size);

			else command(input, par_shell_in_path);
                }
        }
        return EXIT_SUCCESS;
}
