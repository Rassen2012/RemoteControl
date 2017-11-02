#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

#define MAX_BUTTON_TEXT 100

typedef struct Button{
    S_Widget *widget;
    int fontSize;
    char *text;
    char visible;
    char enabled;
    char hover;
    char focus;
    char tab;
    char pressed;

    void *arg;
    void (*Click)(struct Button *, void *);
    void (*Paint)(struct Button *, PaintEventArgs *);
}Button;

Button *Button_newButton(int x, int y, int width, int height, char *text, S_Widget *parent);
void Button_Dispose(Button *b);
void Button_Paint(Button *b, PaintEventArgs *ev);
void Button_Show(Button *b);
void Button_Hide(Button *b);
void Button_Move(Button *b, int x, int y);
void Button_Resize(Button *b, int width, int height);
void Button_SetText(Button *b, char *text);
void Button_Enable(Button *b);
void Button_Disable(Button *b);
void Button_SetTab(Button *b);
void Button_Untab(Button *b);
void Button_SetBackgroundColor(Button *b, Color c);
Color Button_GetBackgroundColor(Button *b);
void Button_SetForegroundColor(Button *b, Color c);
Color Button_GetForegroundColor(Button *b);
void Button_SetBackgroundImage(Button *b, Pixmap image);
Pixmap Button_GetBackgroundImage(Button *b);

void Button_MouseEnter(Button *b, MouseEventArgs *ev);
void Button_MouseLeave(Button *b, MouseEventArgs *ev);
void Button_MousePress(Button *b, MouseEventArgs *ev);
void Button_MouseRelease(Button *b, MouseEventArgs *ev);
void Button_KeyPress(Button *b, KeyEventArgs *ev);
void Button_KeyRelease(Button *b, KeyEventArgs *ev);
void Button_ResizeEvent(Button *b, ResizeEventArgs *ev);
void Button_SetFontSize(Button *b, int size);
void Button_MouseMove(Button *b, MouseEventArgs *ev);
void Button_SetDock(Button *b, int dock);

#endif // BUTTON_H

