#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listview.h"
#include "listviewitem.h"

#define TEXT_X 2
#define TEXT_Y 18
#define DEFAULT_HEIGHT 20

S_ListViewItem *S_ListViewItem_newListViewItem(S_ListView *parent, char *text){
    S_ListViewItem *item = (S_ListViewItem*)malloc(sizeof(S_ListViewItem));
    item->parent = parent;
    item->hover = item->selected = 0;
    item->widget = Widget_newWidget(0, 0, parent->widget->width, DEFAULT_HEIGHT, parent->widget);
    item->widget->type = WIDGET_TYPE_LISTVIEWITEM;
    item->widget->object = (void*)item;
    item->object = NULL;
    item->ItemSelect = NULL;
    item->widget->is_tabed = 0;
    Widget_SetBorderWidth(item->widget, 0);
    XSelectInput(item->widget->display, item->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask);
    if(text){
        item->text = strdup(text);
        int t_width = XTextWidth(item->widget->g->fontInfo, item->text, strlen(item->text));
        if(t_width > item->widget->width){
            t_width += 2;
            Widget_Resize(item->widget, t_width, item->widget->height);
            if(t_width > parent->widget->width) Widget_Resize(parent->widget, t_width, parent->widget->height);
        }
    }
    else item->text = NULL;
    ListView_AddItem(parent, item);
    return item;
}

void S_ListViewItem_Dispose(S_ListViewItem *item){
    item->widget->disposed = 1;
    if(item->parent != NULL) Widget_RemoveChild(item->parent->widget, item->widget);
    free(item);
}

void S_ListViewItem_SetText(S_ListViewItem *item, char *text){
    if(item->text) free(item->text);
    item->text = strdup(text);
    S_ListViewItem_Paint(item, NULL);
}

void S_ListViewItem_Resize(S_ListViewItem *item, int width, int height){
    Widget_Resize(item->widget, width, height);
}

void S_ListViewItem_Move(S_ListViewItem *item, int x, int y){
    Widget_Move(item->widget, x, y);
}

void S_ListViewItem_Paint(S_ListViewItem *item, PaintEventArgs *ev){
    Graphics *g = item->widget->g;
    Color backcolor;
    if(item->selected && !item->parent->widget->tab) backcolor = Color_GetColor1(205, 210, 220, item->widget->display);
    else if(item->selected && item->parent->widget->tab) backcolor = Color_GetColor1(145, 176, 115, g->widget->display);
    else if(item->hover) backcolor = Color_GetColor1(220, 220, 220, item->widget->display);
    else backcolor = Color_GetColor1(250, 250, 250, item->widget->display);
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, backcolor);
    Graphics_FillRectangle(g, 0, 0, item->widget->width, item->widget->height);
    if(item->text){
        Color textcolor;
        if(item->selected/* || item->hover*/) textcolor = Color_GetColor1(255, 255, 255, item->widget->display);
        else textcolor = Color_GetColor1(0, 0, 0, g->widget->display);
        Graphics_SetColor(g, textcolor);
        Graphics_DrawText(g, TEXT_X, TEXT_Y, item->text);
    }
    Graphics_EndPaint(g);
}

void S_ListViewItem_Select(S_ListViewItem *item, char val){
    item->selected = val;
    S_ListViewItem_Paint(item, NULL);
    if(val && item->ItemSelect) item->ItemSelect(item->sender, item->object);
}

void S_ListViewItem_MouseEnter(S_ListViewItem *item, MouseEventArgs *ev){
    item->hover = 1;
    S_ListViewItem_Paint(item, NULL);
}

void S_ListViewItem_MouseLeave(S_ListViewItem *item, MouseEventArgs *ev){
    item->hover = 0;
    S_ListViewItem_Paint(item, NULL);
}

void S_ListViewItem_MouseDown(S_ListViewItem *item, MouseEventArgs *ev){
    Widget_Tab(item->parent->widget);
    if(ev->button == MOUSE_BUTTON_LEFT) ListView_ItemSelect(item->parent, item);
    if(ev->button == MOUSE_BUTTON_LEFT && item->ItemSelect) item->ItemSelect(item->sender, item->object);
}

void S_ListViewItem_ResizeEvent(S_ListViewItem *item, ResizeEventArgs *ev){
    Widget_Resize(item->widget, ev->width-15, item->widget->height);
}

void S_ListViewItem_SetObject(S_ListViewItem *item, void *sender, void *obj){
    item->object = obj;
    item->sender = sender;
}
