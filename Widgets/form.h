#ifndef FORM_H
#define FORM_H

#include <pthread.h>
#include "widget.h"
#include "eventstack.h"
#include "button.h"

typedef enum FormTypes{
    MAIN_FORM,
    DIALOG_FORM
}FormTypes;

typedef enum FormMode{
    FULLSCREEN_MODE,
    WINDOW_MODE
}FormMode;

typedef enum FormCursors{
    FORM_DEFAULT_CURSOR,
    FORM_LEFT_CURSOR,
    FORM_RIGHT_CURSOR,
    FORM_TOP_CURSOR,
    FORM_BOTTOM_CURSOR,
    FORM_BOTTOM_LEFT_CURSOR,
    FORM_BOTTOM_RIGHT_CURSOR,
    FORM_TOP_LEFT_CURSOR,
    FORM_TOP_RIGHT_CURSOR
}FormCursors;

typedef struct Form{
    S_Widget *widget;
    int type;
    struct Form *parent;
    struct Form **childs;

    char visible;
    char *title;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
    Pixmap p;
    XImage *im;
    FormMode mode;
    FormCursors currentCursor;
    Cursor cur;
    char rctrl;

    void *obj;

    EventStack *evStack;
    pthread_mutex_t evs_mutex;

#if defined(OC2K1x)
    Button *btnClose;
    char headPressed;
#endif

    void (*Paint)(struct Form *, PaintEventArgs *);
    void (*MousePress)(struct Form *, MouseEventArgs *);
    void (*MouseRelease)(struct Form *, MouseEventArgs *);
    void (*MouseEnter)(struct Form *);
    void (*MouseLeave)(struct Form *);
    void (*MouseMove)(struct Form *, MouseEventArgs *);
    void (*KeyUp)(struct Form *, KeyEventArgs *);
    void (*KeyDown)(struct Form *, KeyEventArgs *);
    void (*ResizeEvent)(struct Form*, ResizeEventArgs *);
    void (*Close)(struct Form *);
}Form;

Form *Form_newForm(int x, int y, int width, int height, int type, Form *parent);
void Form_Dispose(Form *f);
void Form_Show(Form *f);
void Form_Hide(Form *f);
void Form_SetBackgroundColor(Form *f, Color c);
void Form_SetForegroundColor(Form *f, Color c);
void Form_SetBackgroundImage(Form *f, Pixmap image);
void Form_Move(Form *f, int x, int y);
void Form_Resize(Form *f, int width, int height);
int Form_X(Form *f);
int Form_Y(Form *f);
int Form_Width(Form *f);
int Form_Height(Form *f);
void Form_Paint(Form *f, PaintEventArgs *ev);
void Form_SetTitle(Form *f, char *title);
void Form_SetMinSize(Form *f, int min_width, int min_height);
void Form_SetSizePolicy(Form *f, int min_width, int min_height, int max_width, int max_height);
void Form_AddWidget(Form *f, S_Widget *child);
void Form_RemoveWidget(Form *f, S_Widget *child);
void Form_AddDialog(Form *f, Form *dialog);
void Form_RemoveDialog(Form *f, Form *dialog);
void Form_MouseMove(Form *f, MouseEventArgs *ev);
void Form_MousePress(Form *f, MouseEventArgs *ev);
void Form_MouseRelease(Form *f, MouseEventArgs *ev);
void Form_MouseEnter(Form *f);
void Form_MouseLeave(Form *f);
void Form_KeyPress(Form *f, KeyEventArgs *ev);
void Form_KeyRelease(Form *f, KeyEventArgs *ev);
void Form_ResizeEvent(Form *f, ResizeEventArgs *ev);
void Form_SetWindowMode(Form *f, FormMode mode);
void Form_Close(Form *f);
void Form_DrawBorder(Graphics *g, int width, int height);
void Form_AddEvent(Form *f, Event ev);

#endif // FORM_H

