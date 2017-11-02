#ifndef LABEL_H
#define LABEL_H

#include "graphics.h"

typedef struct Label{
    S_Widget *w;

    int fontSize;
    char *text;
    char visible;
    void (*Paint)(struct Label*, PaintEventArgs *);
}Label;

Label *Label_newLabel(int x, int y, char *text, S_Widget *parent);
void Label_Dispose(Label *lbl);
void Label_Show(Label *lbl);
void Label_Hide(Label *lbl);
void Label_Paint(Label *lbl, PaintEventArgs *ev);
void Label_SetText(Label *lbl, char *text);
void Label_SetBackgroundColor(Label *lbl, Color c);
void Label_SetForegroundColor(Label *lbl, Color c);
void Label_SetBackgroundImage(Label *lbl, Pixmap pic);
Color Label_GetBackgroundColor(Label *lbl);
Color Label_GetForegroundColor(Label *lbl);
Pixmap Label_GetBackgroundImage(Label *lbl);
int Label_X(Label *lbl);
int Label_Y(Label *lbl);
int Label_Width(Label *lbl);
int Label_Height(Label *lbl);
void Label_Move(Label *lbl, int x, int y);
void Label_Resize(Label *lbl, int width, int height);
void Label_ResizeEvent(Label *lbl, ResizeEventArgs *ev);
void Label_SetFontSize(Label *lbl, int size);

#endif // LABEL_H

