#ifndef PARSHELL_SYNC_H
#define PARSHELL_SYNC_H

#include <stdbool.h>
#include "list.h"

bool exit_called(void);

void regist_wait(int pid, time_t endtime);

void regist_fork(int pid, time_t starttime);

void set_exit_called(void);

time_t get_finish_time(int pid);

void* par_wait(void*);

pid_t synced_wait(int* status);

pid_t synced_fork(void); 

void threading_init(list_t*);

void threading_cleanup(void);

#endif

