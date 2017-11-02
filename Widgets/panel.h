#ifndef PANEL_H
#define PANEL_H

#include "graphics.h"

typedef struct Panel{
    S_Widget *widget;

    char hover;
    char selected;
    char active;
    char *title;
    int id;
    XImage *backgroundImage;
    int oldx, oldy;

    void (*MouseMove)(struct Panel *, MouseEventArgs *);
    void (*MousePress)(struct Panel *, MouseEventArgs *);
    void (*MouseRelease)(struct Panel *, MouseEventArgs *);
    void (*KeyDown)(struct Panel *, KeyEventArgs *);
    void (*KeyUp)(struct Panel *, KeyEventArgs *);
    void (*Paint)(struct Panel *, PaintEventArgs *);
}Panel;

Panel *Panel_newPanel(int x, int y, int width, int height, S_Widget *parent);
void Panel_Dispose(Panel *p);
void Panel_Paint(Panel *p, PaintEventArgs *ev);
void Panel_Show(Panel *p);
void Panel_Hide(Panel *p);
void Panel_MouseMove(Panel *p, MouseEventArgs *ev);
void Panel_MousePress(Panel *p, MouseEventArgs *ev);
void Panel_MouseRelease(Panel *p, MouseEventArgs *ev);
void Panel_MouseEnter(Panel *p, MouseEventArgs *ev);
void Panel_MouseLeave(Panel *p, MouseEventArgs *ev);
void Panel_KeyDown(Panel *p, KeyEventArgs *ev);
void Panel_KeyUp(Panel *p, KeyEventArgs *ev);
void Panel_Resize(Panel *p, int width, int height);
void Panel_Move(Panel *p, int x, int y);
void Panel_SetTitle(Panel *p, char *title);
void Panel_Tab(Panel *p);
void Panel_Untab(Panel *p);
void Panel_SetBackgroundImage(Panel *p, XImage *im);

#endif // PANEL_H

