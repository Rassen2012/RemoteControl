#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "s_treeview.h"

TreeView *TreeView_newTreeView(S_Widget *parent, int x, int y, int width, int height){
    if(parent == NULL) return NULL;
    TreeView *tv = (TreeView*)malloc(sizeof(TreeView));
    tv->widget = Widget_newWidget(x, y, width, height, parent);
    tv->widget->type = WIDGET_TYPE_TREEVIEW;
    tv->widget->object = (void*)tv;
    tv->childs = NULL;
    tv->childCount = 0;
    XSelectInput(tv->widget->display, tv->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);
    return tv;
}

void TreeView_Dispose(TreeView *tv){
    if(tv->childCount > 0) free(tv->childs);
    free(tv);
}

void TreeView_Paint(TreeView *tv, PaintEventArgs *ev){
//    int i;
//    for(i = 0; i < tv->childCount; i++){
//        TreeViewItem_Paint(tv->childs[i], NULL);
//    }
    Graphics *g = tv->widget->g;
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, Color_GetColor1(255, 255, 255, g->widget->display));
    Graphics_FillRectangle(g, 0, 0, g->widget->width, g->widget->height);
    Graphics_EndPaint(g);
}
