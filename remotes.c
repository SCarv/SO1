#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "monitor.h"

struct remote {

        pid_t pid;
        char* in_path;
        char* out_path;
        struct remote* next;
};

static struct remote* head = NULL;

struct remote* new_remote(pid_t pid, char* in_path, char* out_path)
{
        struct remote* new = malloc(sizeof(struct remote));
        new->pid = pid;
        new->out_path = strdup(out_path);
        new->in_path = strdup(in_path);

        return new;
}

void insert_remote(struct remote* remote)
{
        remote->next = head;
        head = remote;
}

struct remote* seek_remote(pid_t target)
{
        struct remote* next;
        struct remote* this = head;

        while (this) {
                next = this->next;
                if (this->pid == target) return this;
                else this = next;
        }
        return NULL;
}

void stats(pid_t pid)
{
        struct remote* target = seek_remote(pid);
        FILE* out = fopen(target->out_path, "w");
        fprintf(out,"%d\n%d", get_iteration_count(), (int) get_total_time());
        fclose(out);
}


void acknowledge(char* argv[])
{
        pid_t pid = strtol(argv[0], NULL, 10);
        // argv[1] should say "regist"
	char* in_path = argv[2];
        char* out_path = argv[3];
	puts(in_path); puts(out_path); puts(argv[0]);
        struct remote* new = new_remote(pid, in_path, out_path);
        insert_remote(new);
}

void terminate_remotes()
{
        struct remote* next;
        struct remote* this = head;

        while (this) {
                next = this->next;
                kill(SIGTERM, this->pid);
                free(this->out_path);
                free(this);
                this = next;
        }
}
