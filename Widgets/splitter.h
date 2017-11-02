#ifndef SPLITTER
#define SPLITTER

#include "widget.h"

typedef struct TVSplitter{
    S_Widget *mainWidget;
    S_Widget *leftWindow;
    S_Widget *rightWindow;

    char spacePressed;
    char spaceHover;
    int oldX;
    int oldY;
    double left_width;         //percent
    double right_width;        //percent

    Cursor cur;
}Splitter;

Splitter *Splitter_newSplitter(int x, int y, int width, int height, S_Widget *parent, S_Widget *leftWidget, S_Widget *rightWidget);
void Splitter_Dispose(Splitter *sp);
void Splitter_Paint(Splitter *sp, PaintEventArgs *ev);
void Splitter_MouseMove(Splitter *sp, MouseEventArgs *ev);
void Splitter_MousePress(Splitter *sp, MouseEventArgs *ev);
void Splitter_MouseRelease(Splitter *sp, MouseEventArgs *ev);
void Splitter_MouseEnter(Splitter *sp, MouseEventArgs *ev);
void Splitter_MouseLeave(Splitter *sp, MouseEventArgs *ev);
void Splitter_ResizeEvent(Splitter *sp, ResizeEventArgs *ev);
void Splitter_SetWidgets(Splitter *sp, S_Widget *leftWidget, S_Widget *rightWidget);
void Splitter_SetRightWidget(Splitter *sp, S_Widget *rightWidget);
void Splitter_SetLeftWidget(Splitter *sp, S_Widget *leftWidget);
void Splitter_SetSize(Splitter *sp, int leftPercent);

#endif // SPLITTER

