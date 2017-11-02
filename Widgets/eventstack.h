#ifndef EVENTSTACK
#define EVENTSTACK

enum{
    EV_SET_PANEL,
    EV_REMOVE_PANEL,
    EV_SET_PANEL_IMAGE,
    EV_SET_IMAGE,
    EV_MOVE_PANEL,
    EV_CLOSE_WINDOW,
    EV_RAISE_WINDOW,
    EV_FORM_PAINT
};

typedef struct EVENT{
    int type;
    int param;
    int param1;
    void (*func)(void*arg);
    void *arg;
}Event;

typedef struct EVENT_STACK{
    Event event;
    struct EVENT_STACK *first;
    struct EVENT_STACK *next;
    int count;
}EventStack;

void EventStack_Push(EventStack **es, Event ev);

EventStack* EventStack_Pop(EventStack **es);

void EventStack_Dispose(EventStack **es);

#endif //EVENTSTACK
