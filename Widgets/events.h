#ifndef EVENTS_H
#define EVENTS_H

#include <X11/X.h>

#define MOUSE_BUTTON_LEFT Button1
#define MOUSE_BUTTON_RIGHT Button2
#define MOUSE_BUTTON_MIDLE Button3

//key codes
#define KEY_ENTER 36
#define KEY_ESC 9
#define KEY_TAB 23
#define KEY_SPACE 65
#define KEY_CNTR 37

//break or continue event
#define S_EVENT_BREAK 0
#define S_EVENT_CONTINUE 1

typedef struct {
    Window w;
    void *obj;
    char con;
}EventArgs;

typedef struct {
    int x;
    int y;
    int global_x;
    int global_y;
    int x_prev;
    int y_prev;
    int xg_prev;
    int yg_prev;
    unsigned int button;
    Window win;
    char con;
} MouseEventArgs;

typedef struct {
    int keyCode;
    char keyChar;
    Window win;
    char con;
    KeySym ksym;
    char shift_mod;
    char caps_mod;
    char ctrl_mod;
} KeyEventArgs;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    Window win;
    char con;
} PaintEventArgs;

typedef struct {
    int width;
    int height;
    int old_width;
    int old_height;
    Window win;
    char con;
} ResizeEventArgs;

PaintEventArgs *newPaintEventArgs(int x, int y, int width, int height);
MouseEventArgs *newMouseEventArgs(int x, int y, int global_x, int global_y, unsigned int button, Window win);
KeyEventArgs *newKeyEventArgs(int keycode, char keychar, Window win);
ResizeEventArgs *newResizeEventArgs(int width, int height, int old_width, int old_height, Window win);
EventArgs *newEventArgs(Window w, void *obj);

#endif // EVENTS_H

