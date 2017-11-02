#ifndef SLIDER_H
#define SLIDER_H

#include "widget.h"

typedef enum SLIDER_TYPE{
    SLIDER_TYPE_VSCROLL,
    SLIDER_TYPE_HSCROLL
}SLIDER_TYPE;

typedef void (*Slider_Mov)(void *obj, float val);

typedef struct Slider{
    S_Widget *widget;

    SLIDER_TYPE type;
    int min_size, size;
    float step;
    int slid_x, slid_y;
    int prev_x, prev_y;

    char hover, pressed;
    char max_up, max_down;
    Slider_Mov mov;
    Rectangle top_b;
    Rectangle bottom_b;
}Slider;

Slider *Slider_newSlider(S_Widget *parent, SLIDER_TYPE type, int x, int y, int width, int height, Slider_Mov m);
void Slider_Dispose(Slider *sl);
void Slider_Paint(Slider *sl, PaintEventArgs *ev);
void Slider_MouseMove(Slider *sl, MouseEventArgs *ev);
void Slider_MouseDown(Slider *sl, MouseEventArgs *ev);
void Slider_MouseUp(Slider *sl, MouseEventArgs *ev);
void Slider_MouseLeave(Slider *sl, MouseEventArgs *ev);
void Slider_ResizeEvent(Slider *sl, ResizeEventArgs *ev);
void Slider_SetStep(Slider *sl, float step);
void Slider_SetMinSize(Slider *sl, int min_size);
void Slider_SetSize(Slider *sl, int size);
void Slider_Move(Slider *sl, int val);
void Slider_MovePer(Slider *sl, int per);

#endif // SLIDER_H

