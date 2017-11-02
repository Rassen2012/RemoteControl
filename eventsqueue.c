#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "eventsqueue.h"

void EventsQueue_Push(EventQueue **qu, Events ev){
    EventQueue *tmp;
    tmp = (EventQueue*)malloc(sizeof(EventQueue));
    tmp->ev = ev;
    if((*qu) != NULL){
        tmp->first = (*qu)->first;
        (*qu)->next = tmp;
    }
    else{
        tmp->first = tmp;
    }
    tmp->next = NULL;
    *qu = tmp;
}

int EventsQueue_Pop(EventQueue **qu, Events *ev){
    EventQueue *tmp;
    if((*qu) == NULL){
        return -1;
    }
    tmp = (*qu)->first;
    memcpy(ev, &tmp->ev, sizeof(Events));
    tmp = tmp->next;
    free((*qu)->first);
    (*qu)->first = tmp;
    if(tmp == NULL){
        *qu = NULL;
    }
    return 1;
}

void EventsQueue_Dispose(EventQueue **qu){
    EventQueue *tmp;
    if(*qu == NULL) return;
    tmp = (*qu)->first;
    do{
        *qu = tmp;
        tmp = tmp->next;
        free((*qu));
    }while(tmp != NULL);
}
