#ifndef S_TREEVIEW_H
#define S_TREEVIEW_H

#include "widget.h"
#include "s_treeviewitem.h"

typedef struct TreeView{
    S_Widget *widget;
    TreeViewItem **childs;
    int childCount;
}TreeView;

TreeView *TreeView_newTreeView(S_Widget *parent, int x, int y, int width, int height);
void TreeView_Dispose(TreeView *tv);
void TreeView_Paint(TreeView *tv, PaintEventArgs *ev);

#endif // S_TREEVIEW_H

