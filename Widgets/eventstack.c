#include <stdio.h>
#include <pthread.h>
#ifdef linux
#include <malloc.h>
#endif
#include "eventstack.h"

void EventStack_Push(EventStack **es, Event ev){
    EventStack *tmp;
    tmp = (EventStack*)malloc(sizeof(EventStack));
    tmp->event = ev;
    if((*es) != NULL){
        tmp->first = (*es)->first;
        (*es)->next = tmp;
    }
    else{
        tmp->first = tmp;
    }
    tmp->next = NULL;
    *es = tmp;
}

/*void EventStack_Push(EventStack **es, Event ev)
{
    EventStack *tmp = (EventStack*)malloc(sizeof(EventStack));
    if(tmp == NULL)
    {
        fprintf(stdout, "Out of memory!\n");
        return;
    }
    tmp->next = *es;
    tmp->event = ev;
    *es = tmp;
}*/

EventStack* EventStack_Pop(EventStack **es){
    EventStack *tmp, *out;
    if((*es) == NULL){
        return NULL;
    }
    out = (*es)->first;
    //memcpy(ev, &tmp->ev, sizeof(Events));
    tmp = out->next;
    //free((*qu)->first);
    (*es)->first = tmp;
    if(tmp == NULL){
        *es = NULL;
    }
    return out;
}

/*EventStack* EventStack_Pop(EventStack **es)
{
    EventStack *out;
    if(*es == NULL) return NULL;
    out = *es;
    *es = (*es)->next;
    return out;
}*/

void EventStack_Dispose(EventStack **es){
    EventStack *tmp;
    while((tmp = EventStack_Pop(es)) != NULL){
        free(tmp);
    }
}


