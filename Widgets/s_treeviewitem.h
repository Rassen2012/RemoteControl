#ifndef S_TREEVIEWITEM
#define S_TREEVIEWITEM

#include "widget.h"

struct TreeView;

typedef struct S_TreeViewItem{
    S_Widget *widget;
    char *text;
    int id;
    struct S_TreeViewItem **childs;
    struct TreeView *parent;
    struct S_TreeViewItem *parentItem;
    int childCount;
    char hovered;
    char pressed;
    char wrapped;
    void *object;
    Cursor cur;
}TreeViewItem;

TreeViewItem *TreeViewItem_newTreeViewItem(TreeViewItem *parent, char *txt);
TreeViewItem *TreeViewItem_newTreeViewItem_N(struct TreeView *parent, char *txt);
void TreeViewItem_Dispose(TreeViewItem *item);
void TreeViewItem_Paint(TreeViewItem *item, PaintEventArgs *ev);
void TreeViewItem_MouseMove(TreeViewItem *item, MouseEventArgs *ev);
void TreeViewItem_MouseEnter(TreeViewItem *item, MouseEventArgs *ev);
void TreeViewItem_MouseLeave(TreeViewItem *item, MouseEventArgs *ev);
void TreeViewItem_MousePress(TreeViewItem *item, MouseEventArgs *ev);
void TreeViewItem_MouseRelease(TreeViewItem *item, MouseEventArgs *ev);

#endif // S_TREEVIEWITEM

