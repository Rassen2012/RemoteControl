#include <stdio.h>
#include <stdlib.h>
#include "events.h"

PaintEventArgs *newPaintEventArgs(int x, int y, int width, int height){
    PaintEventArgs *ev = (PaintEventArgs*)malloc(sizeof(PaintEventArgs));
    if(ev == NULL) return NULL;
    ev->x = x;
    ev->y = y;
    ev->width = width;
    ev->height = height;
    return ev;
}

MouseEventArgs *newMouseEventArgs(int x, int y, int global_x, int global_y, unsigned int button, Window win){
    MouseEventArgs *ev = (MouseEventArgs*)malloc(sizeof(MouseEventArgs));
    if(ev == NULL) return NULL;
    ev->x = x;
    ev->y = y;
    ev->global_x = global_x;
    ev->global_y = global_y;
    ev->button = button;
    ev->win = win;
    return ev;
}

KeyEventArgs *newKeyEventArgs(int keycode, char keychar, Window win){
    KeyEventArgs *ev = (KeyEventArgs*)malloc(sizeof(KeyEventArgs));
    if(ev == NULL) return NULL;
    ev->keyCode = keycode;
    ev->keyChar = keychar;
    ev->win = win;
    ev->shift_mod = ev->caps_mod = ev->ctrl_mod = 0;
    return ev;
}

ResizeEventArgs *newResizeEventArgs(int width, int height, int old_width, int old_height, Window win){
    ResizeEventArgs *ev = (ResizeEventArgs*)malloc(sizeof(ResizeEventArgs));
    if(ev == NULL) return NULL;
    ev->width = width;
    ev->height = height;
    ev->old_width = old_width;
    ev->old_height = old_height;
    ev->win = win;
    return ev;
}

EventArgs *newEventArgs(Window w, void *obj){
    EventArgs *ev = (EventArgs*)malloc(sizeof(EventArgs));
    ev->w = w;
    ev->obj = obj;
    ev->con = 1;
    return ev;
}
