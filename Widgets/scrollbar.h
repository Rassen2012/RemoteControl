#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "slider.h"

typedef enum SCROLL_TYPES{
    VERTICAL_SCROLL = 0x01,
    HORIZONTAL_SCROLL = 0x02
}SCROLL_TYPES;

typedef struct Scroll{
    S_Widget *widget;
    int type;
    float v_ost, h_ost;

    Slider *slVer;
    Slider *slHor;
    S_Widget *sw;
}Scroll;

Scroll *Scroll_newScroll(S_Widget *parent, int x, int y, int width, int height, SCROLL_TYPES type);
void Scroll_Dispose(Scroll *s);
void Scroll_Paint(Scroll *s, PaintEventArgs *ev);
void Scroll_ResizeEvent(Scroll *s, ResizeEventArgs *ev);
void Scroll_SetWidget(Scroll *s, S_Widget *sw);

#endif // SCROLLBAR_H

