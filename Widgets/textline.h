#ifndef TEXTLINE_H
#define TEXTLINE_H

#include "widget.h"

typedef struct TextLine{
    S_Widget *widget;
    char *text;
    const char *def_text;
    int cur_pos;
    char hover;
    char focus;
    char cur_visible;
    char radius;
    int text_count;
    int text_pos;
    pthread_t th;
    char draw_cur;
    char stop;
    int shift;
    Cursor defCursor;
    Cursor focCursor;

    void (*EnterPress)(void *obj, char *text);
    void *obj;
}TextLine;

TextLine *TextLine_newTextLine(S_Widget *parent, int x, int y, int width, int height, const char *def_text, char radius);
void TextLine_Dispose(TextLine *tl);
void TextLine_Paint(TextLine *tl, PaintEventArgs *ev);
void TextLine_MouseEnter(TextLine *tl, MouseEventArgs *ev);
void TextLine_MouseLeave(TextLine *tl, MouseEventArgs *ev);
void TextLine_MouseDown(TextLine *tl, MouseEventArgs *ev);
void TextLine_KeyPress(TextLine *tl, KeyEventArgs *ev);
void TextLine_KeyRelease(TextLine *tl, KeyEventArgs *ev);
void TextLine_FocusIn(TextLine *tl, EventArgs *ev);
void TextLine_FocusOut(TextLine *tl, EventArgs *ev);
void TextLine_Tab(TextLine *tl);
void TextLine_Untab(TextLine *tl);

#endif // TEXTLINE_H

