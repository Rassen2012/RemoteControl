#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/cursorfont.h>
#include "s_treeview.h"
#include "s_treeviewitem.h"

#define TI_X 0
#define TI_Y 0
#define TI_X_STEP 5
#define TI_HEIGHT 20
#define Text_X 15
#define Text_Y 18

void TreeViewItem_AddChild(TreeViewItem *parent, TreeViewItem *child);
void TreeView_AddChild(TreeView *parent, TreeViewItem *child);

TreeViewItem *TreeViewItem_newTreeViewItem(TreeViewItem *parent, char *txt){
    TreeViewItem *ti = (TreeViewItem*)malloc(sizeof(TreeViewItem));
    int y = parent->widget->y + parent->childCount*TI_HEIGHT + TI_HEIGHT;
    ti->widget = Widget_newWidget(parent->widget->x+TI_X_STEP, y, parent->widget->width, TI_HEIGHT, parent->parent->widget);
    Widget_AddChild(parent->parent->widget, ti->widget);
    ti->widget->type = WIDGET_TYPE_TREEVIEW_ITEM;
    ti->widget->object = ti;
    ti->childs = NULL;
    ti->childCount = 0;
    ti->wrapped = 0;
    ti->hovered = 0;
    ti->object = NULL;
    ti->pressed = 0;
    ti->parentItem = parent;
    ti->cur = 0;
    if(txt) ti->text = strdup(txt);
    else ti->text = NULL;
    TreeViewItem_AddChild(parent, ti);

    XSelectInput(ti->widget->display, ti->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);

    return ti;
}

TreeViewItem *TreeViewItem_newTreeViewItem_N(TreeView *parent, char *txt){
    TreeViewItem *ti = (TreeViewItem*)malloc(sizeof(TreeViewItem));
    int y = parent->childCount*TI_HEIGHT;
    ti->widget = Widget_newWidget(TI_X, y, parent->widget->width, TI_HEIGHT, parent->widget);
    Widget_AddChild(parent->widget, ti->widget);
    ti->widget->type = WIDGET_TYPE_TREEVIEW_ITEM;
    ti->widget->object = ti;
    ti->childs = NULL;
    ti->object = NULL;
    ti->text = NULL;
    ti->childCount = ti->wrapped = ti->hovered = ti->pressed = ti->cur = 0;
    ti->parent = parent;
    ti->parentItem = NULL;
    if(txt) ti->text = strdup(txt);
    TreeView_AddChild(parent, ti);
    XSelectInput(ti->widget->display, ti->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);
    return ti;
}

void TreeViewItem_Dispose(TreeViewItem *item){
    if(item->text) free(item->text);
    if(item->childCount > 0) free(item->childs);
    free(item);
}

void TreeViewItem_AddChild(TreeViewItem *parent, TreeViewItem *child){
    int size = 0;
    int t_size = sizeof(TreeViewItem*)*2;
    if(parent->childCount > 0){
        size = t_size*parent->childCount;
        TreeViewItem **tmp = (TreeViewItem**)malloc(size);
        memcpy(tmp, parent->childs, size);
        free(parent->childs);
        parent->childs = (TreeViewItem**)malloc(size+t_size);
        memcpy(parent->childs, tmp, size);
        free(tmp);
    }
    else{
        parent->childs = (TreeViewItem**)malloc(t_size);
    }
    parent->childs[parent->childCount++] = child;
}

void TreeView_AddChild(TreeView *parent, TreeViewItem *child){
    int size = 0;
    int t_size = sizeof(TreeViewItem*)*2;
    if(parent->childCount > 0){
        size = t_size*parent->childCount;
        TreeViewItem **tmp = (TreeViewItem**)malloc(size);
        memcpy(tmp, parent->childs, size);
        free(parent->childs);
        parent->childs = (TreeViewItem**)malloc(size+t_size);
        memcpy(parent->childs, tmp, size);
        free(tmp);
    }
    else{
        parent->childs = (TreeViewItem**)malloc(t_size);
    }
    parent->childs[parent->childCount++] = child;
}

void TreeViewItem_Paint(TreeViewItem *item, PaintEventArgs *ev){
    //if(item->parentItem && !item->parentItem->wrapped) return;
    Graphics *g = item->widget->g;
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, Color_GetColor1(255, 255, 255, g->widget->display));
    Graphics_FillRectangle(g, 0, 0, g->widget->width, g->widget->height);

//    if(item->childCount > 0){
//        Graphics_SetColor(g, Color_GetColor1(0, 0, 0, g->widget->display));
//        XPoint point[3];
//        if(!item->wrapped){
//            point[0].x = 2;
//            point[0].y = 18;
//            point[1].x = 2;
//            point[1].y = 8;
//            point[2].x = 12;
//            point[2].y = 12;
//        }
//        else{
//            point[0].x = 2;
//            point[0].y = 8;
//            point[1].x = 12;
//            point[1].y = 8;
//            point[2].x = 7;
//            point[2].y = 18;
//        }
//        Graphics_FillTriangle(g, point);
//    }

    if(item->text){
        if(item->hovered) Graphics_SetColor(g, Color_GetColor1(20, 20, 250, g->widget->display));
        else Graphics_SetColor(g, Color_GetColor1(0, 0, 0, g->widget->display));
        Graphics_DrawText(g, Text_X, Text_Y, item->text);
    }
    Graphics_EndPaint(g);
//    int i;
//    for(i = 0; i < item->childCount; i++){
//        TreeViewItem_Paint(item->childs[i], NULL);
//    }
}

void TreeViewItem_MouseEnter(TreeViewItem *item, MouseEventArgs *ev){
    item->hovered = 1;
    if(!item->cur){
        item->cur = XCreateFontCursor(item->widget->display, XC_hand1);
        //XDefineCursor(item->widget->display, item->widget->window, item->cur);
    }
    XDefineCursor(item->widget->display, item->widget->window, item->cur);
    TreeViewItem_Paint(item, NULL);
}

void TreeViewItem_MouseLeave(TreeViewItem *item, MouseEventArgs *ev){
    item->hovered = 0;
    if(item->cur){
        XUndefineCursor(item->widget->display, item->widget->window);
        //item->cur = 0;
    }
    TreeViewItem_Paint(item, NULL);
}

void TreeViewItem_MouseMove(TreeViewItem *item, MouseEventArgs *ev){

}

void TreeViewItem_MousePress(TreeViewItem *item, MouseEventArgs *ev){
//    if(item->childCount > 0){
//        if(item->wrapped) item->wrapped = 0;
//        else item->wrapped = 1;
//        TreeView_Paint(item->parent, NULL);
//    }
}

void TreeViewItem_MouseRelease(TreeViewItem *item, MouseEventArgs *ev){

}
