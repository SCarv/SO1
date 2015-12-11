/*
 * list.c - implementation of the integer list functions 
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "list.h"



list_t* lst_new()
{
   list_t *list;
   list = (list_t*) malloc(sizeof(list_t));
   list->first = NULL;
   return list;
}


void lst_destroy(list_t *list)
{
	struct lst_iitem *item, *nextitem;

	if (!list) return;

	item = list->first;
	while (item) {
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
	free(list);
}

time_t get_process_time(list_t *list, int pid)
{
	lst_iitem_t *item;
	item = list->first;
	while(item->pid != pid){
		item = item->next;
	}
	return item->endtime - item->starttime;
}

void insert_new_process(list_t *list, int pid, time_t starttime)
{
	lst_iitem_t *item;

	item = (lst_iitem_t *) malloc(sizeof(lst_iitem_t));
	item->pid = pid;
	item->starttime = starttime;
	item->endtime = 0;
	item->next = list->first;
	list->first = item;
}

void update_terminated_process(list_t *list, int pid, time_t endtime)
{
	lst_iitem_t *item;
	item = list->first;
	while(item->pid != pid){
		item = item->next;	
	}	   
	item->endtime = endtime;
}


void lst_print(list_t *list)
{
	lst_iitem_t *item;
	int totaltime;

	if (!list) return;

	item = list->first;
	while (item) {
		totaltime = item->endtime - item->starttime;
		printf("Process %u finished with execution time: %d s\n", item->pid, totaltime);
		item = item->next;
	}
}
