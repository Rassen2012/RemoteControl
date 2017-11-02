#ifndef EVENTSQUEUE_H
#define EVENTSQUEUE_H

#include "public.h"

typedef struct EventQueue{
    Events ev;
    struct EventQueue *first;
    struct EventQueue *next;
    int count;
}EventQueue;

void EventsQueue_Push(EventQueue **qu, Events ev);
int EventsQueue_Pop(EventQueue **qu, Events *ev);
void EventsQueue_Dispose(EventQueue **qu);

#endif // EVENTSQUEUE_H
