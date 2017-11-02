#ifndef DOCUMENTVIEWER_H
#define DOCUMENTVIEWER_H

#include "../Logic/xdocument.h"
#include "widget.h"
#include "scrollbar.h"

//typedef struct TextLine{
//    char *text;
//    int chars;
//}TextLine;

typedef struct LinkRect{
    Rectangle rect;
    char *href;
    char hov;
}LinkRect;

typedef struct DocumentView{
    S_Widget *widget;
    Scroll *s;


    short text_interval;
    short par_interval;
    short font_size;
    short indent_left, indent_right;
    int cur_x, cur_y;
    int lines;
    int cur_line;
    int doc_width;
    char abzac;
    XDocument *doc;
    XDPage *page;
    //TextLine *text_lines;
    char drawing;
    char find;
    XDParagraph *fpar;
    char *ftext;
    int fy;
    int fj;
    int fi;
    int fp;
    char focus;
    char hand;
    int ly;
    XDParagraph *lpar;

    LinkRect *links;
    int links_count;

    Cursor defCur;
    Cursor handCur;

    void *lv;
    void (*SelectItem)(void *lv, char *link);
}DocumentView;

DocumentView *DocumentView_newDocumentView(S_Widget *parent, int x, int y, int width, int height);
void DocumentView_Dispose(DocumentView *dv);
void DocumentView_SetDocument(DocumentView *dv, XDocument *doc);
void DocumentView_SetPage(DocumentView *dv, XDPage *p);
void DocumentView_Paint(DocumentView *dv, PaintEventArgs *ev);
void DocumentView_SetFontSize(DocumentView *dv, short fsize);
void DocumentView_LoadDocument(DocumentView *dv, char *filename);
void DocumentView_SetPageFromId(DocumentView *dv, char *id);
void DocumentView_SetPageEvent(void *sender, void *link);
void DocumentView_Find(void *object, char *text);
void DocumentView_Tab(DocumentView *dv);
void DocumentView_Untab(DocumentView *dv);
void DocumentView_KeyPress(DocumentView *dv, KeyEventArgs *ev);
void DocumentView_MousePress(DocumentView *dv, MouseEventArgs *ev);
void DocumentView_MouseMove(DocumentView *dv, MouseEventArgs *ev);

#endif // DOCUMENTVIEWER_H

