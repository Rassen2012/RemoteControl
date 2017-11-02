#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "button.h"
#include "scrollbar.h"
#include "listviewitem.h"

typedef struct S_ListView{
    S_Widget *widget;
    S_ListViewItem **items;
    int items_count;
    S_ListViewItem *selectedItem;

    Scroll *s;
    char focus;
}S_ListView;

S_ListView *ListView_newListView(S_Widget *parent, int x, int y, int width, int height);
void ListView_Dispose(S_ListView *lv);
void ListView_Paint(S_ListView *lv, PaintEventArgs *ev);
void ListView_ItemSelect(S_ListView *lv, S_ListViewItem *item);
void ListView_AddItem(S_ListView *lv, S_ListViewItem *item);
void ListView_RemoveItem(S_ListView *lv, S_ListViewItem *item);
void ListView_ResizeEvent(S_ListView *lv, ResizeEventArgs *ev);
void ListView_ItemSelectLink(S_ListView *lv, char *link);
void ListView_KeyPress(S_ListView *lv, KeyEventArgs *ev);
void ListView_Tab(S_ListView *lv);
void ListView_Untab(S_ListView *lv);
void ListView_SetPrev(Button *b, void *vlv);
void ListView_SetNext(Button *b, void *vlv);
void ListView_SetStart(Button *b, void *vlv);
void ListView_SetEnd(Button *b, void *vlv);

#endif // LISTVIEW_H

