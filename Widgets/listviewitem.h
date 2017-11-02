#ifndef LISTVIEWITEM_H
#define LISTVIEWITEM_H

#include "widget.h"

struct S_ListView;

typedef struct S_ListViewItem{
    S_Widget *widget;
    char *text;
    char hover, selected;

    struct S_ListView *parent;
    void *object;
    void *sender;
    void (*ItemSelect)(void *sender, void *object);
}S_ListViewItem;

S_ListViewItem *S_ListViewItem_newListViewItem(struct S_ListView *parent, char *text);
void S_ListViewItem_Dispose(S_ListViewItem *item);
void S_ListViewItem_SetText(S_ListViewItem *item, char *text);
void S_ListViewItem_Resize(S_ListViewItem *item, int width, int height);
void S_ListViewItem_Move(S_ListViewItem *item, int x, int y);
void S_ListViewItem_Paint(S_ListViewItem *item, PaintEventArgs *ev);
void S_ListViewItem_Select(S_ListViewItem *item, char val);
void S_ListViewItem_MouseEnter(S_ListViewItem *item, MouseEventArgs *ev);
void S_ListViewItem_MouseLeave(S_ListViewItem *item, MouseEventArgs *ev);
void S_ListViewItem_MouseDown(S_ListViewItem *item, MouseEventArgs *ev);
void S_ListViewItem_ResizeEvent(S_ListViewItem *item, ResizeEventArgs *ev);
void S_ListViewItem_SetObject(S_ListViewItem *item, void *sender, void *obj);

#endif // LISTVIEWITEM_H

