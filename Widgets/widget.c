#include <stdio.h>
#include <string.h>
#ifdef linux
#include <malloc.h>
#endif
#include "widget.h"
#include "form.h"
#include "button.h"
#include "label.h"
#include "panel.h"
#include "splitter.h"
#include "s_treeview.h"
#include "listview.h"
#include "listviewitem.h"
#include "slider.h"
#include "documentviewer.h"
#include "textline.h"

//#define WIDGET_BACKGROUND_COLOR Color_GetColor(255, 255, 255)
//#define WIDGET_FOREGROUND_COLOR Color_GetColor(0, 0, 0)

typedef struct WidgetManager{
    S_Widget *parent;
    int curTab;
    int tabCount;
}WidgetManager;

typedef struct WidgetManagers{
    WidgetManager *m;
    int manager_count;
}WidgetManagers;

WidgetManagers manager = {0};

void Manager_SetManager(S_Widget *parent){
    //WidgetManager *tmp;
    if(manager.manager_count > 0){
//        tmp = (WidgetManager*)malloc(sizeof(WidgetManager)*manager.manager_count);
//        memcpy(tmp, manager.m, sizeof(WidgetManager)*manager.manager_count);
//        free(manager.m);
//        manager.m = (WidgetManager*)malloc(sizeof(WidgetManager)*(manager.manager_count+1));
//        memcpy(manager.m, tmp, sizeof(WidgetManager)*manager.manager_count);
//        free(tmp);
        manager.m = (WidgetManager*)realloc(manager.m, sizeof(WidgetManager)*(manager.manager_count+1));
    }
    else{
        manager.m = (WidgetManager*)malloc(sizeof(WidgetManager));
    }
    manager.m[manager.manager_count].parent = parent;
    manager.m[manager.manager_count].curTab = 0;
    manager.m[manager.manager_count].tabCount = 0;
    ++manager.manager_count;
}

void Manager_RemoveManager(Window id){
//    int i, n = 0;
//    WidgetManager *tmp;
//    for(i = 0; i < manager.manager_count; i++){
//        if(manager.m[i].id == id){
//            if(manager.manager_count == 1){
//                manager.manager_count--;
//                free(manager.m);
//                return;
//            }
//            tmp = (WidgetManager*)malloc(sizeof(WidgetManager)*manager.manager_count);
//            memcpy(tmp, manager.m, sizeof(WidgetManager)*manager.manager_count);
//            free(manager.m);
//            manager.m = (WidgetManager*)malloc(sizeof(WidgetManager)*(manager.manager_count-1));
//            for(i = 0; i < manager.manager_count; i++){
//                if(tmp[i].id == id) continue;
//                memcpy(&manager.m[n++], &tmp[i], sizeof(WidgetManager));
//            }
//            free(tmp);
//            manager.manager_count--;
//            return;
//        }
//    }
}

int Manager_SelectTab(S_Widget *id){
    int i = 0;
    for(i = 0; i < manager.manager_count; i++){
        if(manager.m[i].parent == id){
            manager.m[i].curTab++;
            if(manager.m[i].curTab >= manager.m[i].tabCount) manager.m[i].curTab = 0;
            return manager.m[i].curTab;
        }
    }
    return 0;
}

int Manager_ReselectTab(S_Widget *w){
    int i = 0;
    for(i = 0; i < manager.manager_count; i++){
        if(manager.m[i].parent == w){
            manager.m[i].curTab--;
            if(manager.m[i].curTab < 0) manager.m[i].curTab = manager.m[i].tabCount - 1;
            return manager.m[i].curTab;
        }
    }
    return 0;
}

void Widget_SetTabIndex(S_Widget *w, int index){
    w->is_tabed = 1;
    w->tabIndex = index;
    S_Widget *parent;
    while((parent = w->parent) != NULL) w = parent;
    int i;
    for(i = 0; i < manager.manager_count; i++){
        if(manager.m[i].parent == w){
            manager.m[i].tabCount++;
        }
    }
}

//int Manager_GetTab(Window id){
//    int i;
//    for(i = 0; i < manager.manager_count; i++){
//        if(manager.m[i].id == id){
//            return ++manager.m[i].tabCount;
//        }
//    }
//    return 0;
//}

S_Widget *Widget_newWidget(int x, int y, int width, int height, S_Widget *parent){
    S_Widget *w = (S_Widget*)malloc(sizeof(S_Widget));
    if(w == NULL) return NULL;
    w->x = x;
    w->y = y;
    w->width = width;
    w->height = height;
    w->parent = parent;
    w->childs = NULL;
    w->focus = 0;
    w->visibility = 1;
    w->visible = 0;
    w->hover = 0;
    w->tab = 0;
    w->is_tabed = 0;
    w->dock = DOCK_LEFT_TOP;
    w->backgroundImage = 0;
    w->child_count = 0;
    w->disposed = 0;
    w->tabIndex = -1;
    if(parent == NULL){
        if((w->display = XOpenDisplay(NULL)) == NULL){
            free(w);
            return NULL;
        }
        Color_SetDepth(DefaultDepth(w->display, DefaultScreen(w->display)), w->display);
        //w->backgroundColor = WIDGET_BACKGROUND_COLOR;
        //w->foregroundColor = WIDGET_FOREGROUND_COLOR;
        w->window = XCreateSimpleWindow(w->display, RootWindow(w->display, DefaultScreen(w->display)), w->x, w->y, w->width, w->height, 0,
                                        BlackPixel(w->display, DefaultScreen(w->display)), WhitePixel(w->display, DefaultScreen(w->display)));
        Manager_SetManager(w);
    }
    else{
        w->display = parent->display;
        w->window = XCreateSimpleWindow(w->display, w->parent->window, w->x, w->y, w->width, w->height, 0,
                                        BlackPixel(w->display, DefaultScreen(w->display)), WhitePixel(w->display, DefaultScreen(w->display)));
        Widget_AddChild(parent, w);
    }
    //w->backgroundColor = WIDGET_BACKGROUND_COLOR;
    //w->foregroundColor = WIDGET_FOREGROUND_COLOR;
    w->backgroundColor = WHITE_COLOR(w->display);
    w->foregroundColor = BLACK_COLOR(w->display);
    w->g = Graphics_newGraphics(w);
    if(w->g == NULL){
        XDestroyWindow(w->display, w->window);
        free(w);
        return NULL;
    }
    return w;
}

void Widget_Dispose(S_Widget *w){
    int i;
    if(w == NULL) return;
    int a;
    for(i = 0; i < w->child_count; i++){
        a = w->child_count;
        Widget_Dispose(w->childs[i]);
        if(w->child_count < a) i--;
    }
    Graphics_Dispose(w->g);
    if(w->backgroundImage != 0){
        XFreePixmap(w->display, w->backgroundImage);
    }
    XDestroyWindow(w->display, w->window);
    if(w->parent == NULL){
        XCloseDisplay(w->display);
    }
    if(!w->disposed){
        w->disposed = 1;
        switch(w->type){
        case WIDGET_TYPE_BUTTON:
            Button_Dispose((Button*)w->object);
            break;
        case WIDGET_TYPE_LABEL:
            Label_Dispose((Label*)w->object);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_Dispose((Panel*)w->object);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_Dispose((Splitter*)w->object);
            break;
        case WIDGET_TYPE_TREEVIEW:
            TreeView_Dispose((TreeView*)w->object);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_Dispose((TreeViewItem*)w->object);
            break;
        case WIDGET_TYPE_LISTVIEW:
            ListView_Dispose((S_ListView*)w->object);
            break;
        case WIDGET_TYPE_LISTVIEWITEM:
            S_ListViewItem_Dispose((S_ListViewItem*)w->object);
            break;
        case WIDGET_TYPE_SLIDER:
            Slider_Dispose((Slider*)w->object);
            break;
        case WIDGET_TYPE_DOCVIEW:
            DocumentView_Dispose((DocumentView *)w->object);
            break;
        case WIDGET_TYPE_TEXTLINE:
            TextLine_Dispose((TextLine*)w->object);
            break;
        }
    }
    free(w);
    w = NULL;
}

void Widget_AddChild(S_Widget *w, S_Widget *child){
//    S_Widget **tmp;
//    if(w->child_count > 0){
//        tmp = (S_Widget**)malloc(sizeof(S_Widget*)*w->child_count);
//        memcpy(tmp, w->childs, sizeof(S_Widget*)*w->child_count);
//        free(w->childs);
//        w->childs = (S_Widget**)malloc(sizeof(S_Widget*)*(w->child_count+1));
//        memcpy(w->childs, tmp, sizeof(S_Widget*)*w->child_count);
//        free(tmp);
//    }
//    else{
//        w->childs = (S_Widget**)malloc(sizeof(S_Widget*));
//    }
//    if(child->is_tabed)
//        child->tabIndex = Manager_GetTab(w->window);
    w->childs = (S_Widget**)realloc(w->childs, sizeof(S_Widget*)*(w->child_count+1));
    w->childs[w->child_count++] = child;
}

void Widget_RemoveChild(S_Widget *w, S_Widget *child){
    if(child == NULL) return;
    S_Widget **tmp = NULL;
    int i;//, n;
    for(i = 0; i < w->child_count; i++){
        if(w->childs[i]->window == child->window){
            S_Widget *tt = w->childs[i];
            if(!child->disposed) Widget_Dispose(tt);
            if(w->child_count - 1 > 0){
                tmp = (S_Widget**)malloc(sizeof(S_Widget*)*(w->child_count-1));
                memcpy(tmp, w->childs, sizeof(S_Widget*)*i);
                memcpy(&tmp[i], &w->childs[i+1], sizeof(S_Widget*)*(w->child_count - (i+1)));
            }
            free(w->childs);
            w->childs = tmp;
            /*w->childs = (S_Widget**)malloc(sizeof(S_Widget*)*(w->child_count-1));
            for(i = 0, n = 0; i < w->child_count; i++){
                if(tmp[i]->window == child->window){
                    Widget_Dispose(child);
                    continue;
                }
                w->childs[n++] = tmp[i];
            }
            free(tmp);*/
            w->child_count--;
            return;
        }
    }
}

void Widget_Show(S_Widget *w){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_Show(w->childs[i]);
    }
    if(w->visibility) {
        XMapWindow(w->display, w->window);
        w->visible = 1;
    }
}

void Widget_Hide(S_Widget *w){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_Hide(w->childs[i]);
    }
    XUnmapWindow(w->display, w->window);
    w->visible = 0;
}

void Widget_Resize(S_Widget *w, int width, int height){
    ResizeEventArgs rev;
    rev.old_width = w->width;
    rev.old_height = w->height;
    rev.width = width;
    rev.height = height;
    rev.con = 1;
    rev.win = w->window;
    w->width = width;
    w->height = height;
    if(w->type != WIDGET_TYPE_FORM) XResizeWindow(w->display, w->window, width, height);
    Graphics_Resize(w->g, &rev);
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_ResizeEvent(w->childs[i], &rev);
    }
}

void Widget_Move(S_Widget *w, int x, int y){
    w->x = x;
    w->y = y;
    XMoveWindow(w->display, w->window, x, y);
}

//S_Widget *Widget_SelectTab(S_Widget *w, int index){
//    int i;
//    S_Widget *tmp;
//    if(index == w->tabIndex) return w;
//    for(i = 0; i < w->child_count; i++){
//        if((tmp = Widget_SelectTab(w->childs[i], index)) != NULL){
//            if(!tmp->is_tabed) continue;
//            return tmp;
//        }
//    }
//    if(index == w->tabIndex) return w;
//    return NULL;
//}

void Widget_Untab(S_Widget *w){
//    int i;
//    for(i = 0; i < parent->child_count; i++){
//        if(parent->childs[i]->tab){
//            parent->childs[i]->tab = 0;
//            switch(parent->childs[i]->type){
//            case WIDGET_TYPE_BUTTON:
//                Button_Untab((Button*)parent->childs[i]->object);
//                break;
//            }
//        }
//    }
    if(!w->tab) return;
    w->tab = 0;
    switch(w->type){
    case WIDGET_TYPE_BUTTON:
        Button_Untab((Button*)w->object);
        break;
    case WIDGET_TYPE_TEXTLINE:
        TextLine_Untab((TextLine*)w->object);
        break;
    case WIDGET_TYPE_LISTVIEW:
        ListView_Untab((S_ListView*)w->object);
        break;
    case WIDGET_TYPE_DOCVIEW:
        DocumentView_Untab((DocumentView*)w->object);
        break;
    case WIDGET_TYPE_PANEL:
        Panel_Untab((Panel*)w->object);
        break;
    default: break;
    }
}

void Widget_ClickTab(S_Widget *w, int x, int y){
    if(!w->is_tabed){
        int i;
        for(i = 0; i < w->child_count; i++){
            Widget_ClickTab(w->childs[i], x - w->x, y - w->y);
        }
    }
    else{
        //if(w->x < x && w->x+w->width > x && w->y < y && w->y+w->height > y) Widget_Tab(w);
        //else if(w->tab) Widget_Untab(w);
        //if(w->x <= x && w->x + w->width >= x && w->y <= y && w->y + w->height >= y) Widget_Tab(w);
        Widget_Tab(w);
    }
}

void Widget_SelectTab(S_Widget *parent, int index){
    if(parent->is_tabed && parent->tabIndex == index) Widget_Tab(parent);
    else if(parent->is_tabed && parent->tab) Widget_Untab(parent);
    int i;
    for(i = 0; i < parent->child_count; i++) Widget_SelectTab(parent->childs[i], index);
}

void Widget_NextTab(S_Widget *parent){
    int index = Manager_SelectTab(parent);
    Widget_SelectTab(parent, index);
}

void Widget_PrevTab(S_Widget *parent){
    int  index = Manager_ReselectTab(parent);
    Widget_SelectTab(parent, index);
}

void Widget_Untabs(S_Widget *parent){
   int i;
   if(parent->is_tabed && parent->tab) Widget_Untab(parent);
   for(i = 0; i < parent->child_count; i++){
       Widget_Untabs(parent->childs[i]);
   }
}

void Widget_Tab(S_Widget *w){
//    int tabIndex;
//    S_Widget *parent = w;
//    S_Widget *tmp = w;
//    while((tmp = tmp->parent) != NULL) parent = tmp;
//    Widget_Untab(parent);
//    tabIndex = Manager_SelectTab(parent->window);
//    tmp = Widget_SelectTab(w, tabIndex);
//    if(tmp == NULL) return;
//    tmp->tab = 1;
    Widget_Untabs(manager.m[0].parent);
    w->tab = 1;
    manager.m[0].curTab = w->tabIndex;
    switch(w->type){
    case WIDGET_TYPE_BUTTON:
        Button_SetTab((Button*)w->object);
        break;
    case WIDGET_TYPE_PANEL:
        Panel_Tab((Panel*)w->object);
        break;
    case WIDGET_TYPE_TEXTLINE:
        TextLine_Tab((TextLine*)w->object);
        break;
    case WIDGET_TYPE_LISTVIEW:
        ListView_Tab((S_ListView*)w->object);
        break;
    case WIDGET_TYPE_DOCVIEW:
        DocumentView_Tab((DocumentView*)w->object);
        break;
    default:
        break;
    }
}

void Widget_Paint(S_Widget *w, PaintEventArgs *ev){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_Paint(w->childs[i], ev);
        if(!ev->con) return;
    }
    if(w->window == ev->win){
        switch(w->type){
        case WIDGET_TYPE_FORM:
            Form_Paint((Form*)w->object, ev);
            break;
        case WIDGET_TYPE_BUTTON:
            Button_Paint((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_LABEL:
            Label_Paint((Label*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_Paint((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_Paint((Splitter*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW:
            TreeView_Paint((TreeView*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_Paint((TreeViewItem*)w->object, ev);
            break;
        case WIDGET_TYPE_LISTVIEW:
            ListView_Paint((S_ListView*)w->object, ev);
            break;
        case WIDGET_TYPE_LISTVIEWITEM:
            S_ListViewItem_Paint((S_ListViewItem*)w->object, ev);
            break;
        case WIDGET_TYPE_SCROLLBAR:
            Scroll_Paint((Scroll*)w->object, ev);
            break;
        case WIDGET_TYPE_SLIDER:
            Slider_Paint((Slider*)w->object, ev);
            break;
        case WIDGET_TYPE_DOCVIEW:
            DocumentView_Paint((DocumentView *)w->object, ev);
            break;
        case WIDGET_TYPE_TEXTLINE:
            TextLine_Paint((TextLine*)w->object, ev);
            break;
        default:break;
        }
        ev->con = S_EVENT_BREAK;
    }
}

void Widget_KeyPressEvent(S_Widget *w, KeyEventArgs *ev){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_KeyPressEvent(w->childs[i], ev);
        if(!ev->con) return;
    }
    if(w->window == ev->win){
        switch(w->type){
        case WIDGET_TYPE_FORM:
            //Form_KeyPress((Form*)w->object, ev);
            break;
        case WIDGET_TYPE_BUTTON:
            Button_KeyPress((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_KeyDown((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_TEXTLINE:
            TextLine_KeyPress((TextLine*)w->object, ev);
            break;
        case WIDGET_TYPE_LISTVIEW:
            ListView_KeyPress((S_ListView*)w->object, ev);
            break;
        case WIDGET_TYPE_DOCVIEW:
            DocumentView_KeyPress((DocumentView*)w->object, ev);
            break;
        default:
            break;
        }
    }
}

void Widget_KeyReleaseEvent(S_Widget *w, KeyEventArgs *ev){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_KeyReleaseEvent(w->childs[i], ev);
        if(!ev->con) return;
    }
    //if(w->window == ev->win){
        switch(w->type){
        case WIDGET_TYPE_FORM:
            Form_KeyRelease((Form*)w->object, ev);
            break;
        case WIDGET_TYPE_BUTTON:
            Button_KeyRelease((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_KeyUp((Panel*)w->object, ev);
            break;
        default:
            break;
        }
    //}
}

void Widget_MouseEnterEvent(S_Widget *w, MouseEventArgs *ev){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_MouseEnterEvent(w->childs[i], ev);
        if(!ev->con) return;
    }
    if(w->window == ev->win){
        switch(w->type){
	case WIDGET_TYPE_FORM:
	    Form_MouseEnter((Form*)w->object);
	    break;
        case WIDGET_TYPE_BUTTON:
            Button_MouseEnter((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_MouseEnter((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_MouseEnter((Splitter*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_MouseEnter((TreeViewItem*)w->object, ev);
            break;
        case WIDGET_TYPE_LISTVIEWITEM:
            S_ListViewItem_MouseEnter((S_ListViewItem*)w->object, ev);
            break;
        case WIDGET_TYPE_TEXTLINE:
            TextLine_MouseEnter((TextLine*)w->object, ev);
            break;
        default:
            break;
        }
    }
}

void Widget_MouseLeaveEvent(S_Widget *w, MouseEventArgs *ev){
    int i;
    for(i = 0; i < w->child_count; i++){
        Widget_MouseLeaveEvent(w->childs[i], ev);
        if(!ev->con) return;
    }
    if(w->window == ev->win){
        switch(w->type){
	case WIDGET_TYPE_FORM:
	    Form_MouseLeave((Form*)w->object);
	    break;
        case WIDGET_TYPE_BUTTON:
            Button_MouseLeave((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_MouseLeave((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_MouseLeave((Splitter*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_MouseLeave((TreeViewItem*)w->object, ev);
            break;
        case WIDGET_TYPE_LISTVIEWITEM:
            S_ListViewItem_MouseLeave((S_ListViewItem*)w->object, ev);
            break;
//        case WIDGET_TYPE_SCROLLBAR:
//            Scroll_MouseLeave((Scroll*)w->object, ev);
//            break;
        case WIDGET_TYPE_SLIDER:
            Slider_MouseLeave((Slider*)w->object, ev);
            break;
        case WIDGET_TYPE_TEXTLINE:
            TextLine_MouseLeave((TextLine*)w->object, ev);
            break;
        default:
            break;
        }
    }
}

void Widget_MouseMoveEvent(S_Widget *w, MouseEventArgs *ev){
    int i;
    if(w->window == ev->win){
        switch(w->type){
        case WIDGET_TYPE_FORM:
            Form_MouseMove((Form*)w->object, ev);
            break;
        case WIDGET_TYPE_BUTTON:
            Button_MouseMove((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_MouseMove((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_MouseMove((Splitter*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_MouseMove((TreeViewItem*)w->object, ev);
            break;
//        case WIDGET_TYPE_SCROLLBAR:
//            Scroll_MouseMove((Scroll*)w->object, ev);
//            break;
        case WIDGET_TYPE_SLIDER:
            Slider_MouseMove((Slider*)w->object, ev);
            break;
        case WIDGET_TYPE_DOCVIEW:
            DocumentView_MouseMove((DocumentView*)w->object, ev);
            break;
        default:
            break;
        }
    }
    for(i = 0; i < w->child_count; i++){
        if(!ev->con) return;
        Widget_MouseMoveEvent(w->childs[i], ev);
    }
}

void Widget_FocusIn(S_Widget *w, EventArgs *ev){
    if(w->window == ev->w){
        ev->con = 0;
        switch(w->type){
        case WIDGET_TYPE_TEXTLINE:
            TextLine_FocusIn((TextLine*)w->object, ev);
            break;
        default: break;
        }
    }
    else{
        int i;
        for(i = 0; i < w->child_count; i++){
            if(!ev->con) return;
            Widget_FocusIn(w->childs[i], ev);
        }
    }
}

void Widget_FocusOut(S_Widget *w, EventArgs *ev){
    if(w->window == ev->w){
        ev->con = 0;
        switch(w->type){
        case WIDGET_TYPE_TEXTLINE:
            TextLine_FocusOut((TextLine*)w->object, ev);
            break;
        default:break;
        }
    }
    else{
        int i;
        for(i = 0; i < w->child_count; i++){
            if(!ev->con) return;
            Widget_FocusOut(w->childs[i], ev);
        }
    }
}

void Widget_MousePressEvent(S_Widget *w, MouseEventArgs *ev){
    int i;
    if(w->window == ev->win){
        switch(w->type){
        case WIDGET_TYPE_FORM:
            Form_MousePress((Form*)w->object, ev);
            break;
        case WIDGET_TYPE_BUTTON:
            Button_MousePress((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_MousePress((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_MousePress((Splitter*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_MousePress((TreeViewItem*)w->object, ev);
            break;
        case WIDGET_TYPE_LISTVIEWITEM:
            S_ListViewItem_MouseDown((S_ListViewItem*)w->object, ev);
            break;
//        case WIDGET_TYPE_SCROLLBAR:
//            Scroll_MouseDown((Scroll*)w->object, ev);
//            break;
        case WIDGET_TYPE_SLIDER:
            Slider_MouseDown((Slider*)w->object, ev);
            break;
//        case WIDGET_TYPE_TEXTLINE:
//            TextLine_MouseDown((TextLine*)w->object, ev);
//            break;
        case WIDGET_TYPE_DOCVIEW:
            DocumentView_MousePress((DocumentView*)w->object, ev);
            break;
        default:
            break;
        }
    }
    if(w->type == WIDGET_TYPE_TEXTLINE) TextLine_MouseDown((TextLine*)w->object, ev);
    for(i = 0; i < w->child_count; i++){
        if(!ev->con) return;
        Widget_MousePressEvent(w->childs[i], ev);
    }
}

void Widget_MouseReleaseEvent(S_Widget *w, MouseEventArgs *ev){
    int i;
    if(w->window == ev->win){
        switch(w->type){
        case WIDGET_TYPE_FORM:
            Form_MouseRelease((Form*)w->object, ev);
            break;
        case WIDGET_TYPE_BUTTON:
            Button_MouseRelease((Button*)w->object, ev);
            break;
        case WIDGET_TYPE_PANEL:
            Panel_MouseRelease((Panel*)w->object, ev);
            break;
        case WIDGET_TYPE_SPLITTER:
            Splitter_MouseRelease((Splitter*)w->object, ev);
            break;
        case WIDGET_TYPE_TREEVIEW_ITEM:
            TreeViewItem_MouseRelease((TreeViewItem*)w->object, ev);
            break;
//        case WIDGET_TYPE_SCROLLBAR:
//            Scroll_MouseUp((Scroll*)w->object, ev);
//            break;
        case WIDGET_TYPE_SLIDER:
            Slider_MouseUp((Slider*)w->object, ev);
            break;
        default:
            break;
        }
    }
    for(i = 0; i < w->child_count; i++){
        if(!ev->con) return;
        Widget_MouseReleaseEvent(w->childs[i], ev);
    }
}

//void Widget_GetTabIndex(S_Widget *w){
//    S_Widget *tmp = w;
//    while((tmp = tmp->parent) != NULL);
//    w->tabIndex = Manager_GetTab(tmp->window);
//    w->is_tabed = 1;
//}

void Widget_ResizeEvent(S_Widget *w, ResizeEventArgs *ev){
//    int i;
//    for(i = 0; i < w->child_count; i++){
//        Widget_ResizeEvent(w->childs[i], ev);
//    }
    switch(w->type){
    case WIDGET_TYPE_FORM:
        Form_ResizeEvent((Form *)w->object, ev);
        break;
    case WIDGET_TYPE_BUTTON:
        Button_ResizeEvent((Button*)w->object, ev);
        break;
    case WIDGET_TYPE_SPLITTER:
        Splitter_ResizeEvent((Splitter*)w->object, ev);
        break;
    case WIDGET_TYPE_LISTVIEW:
        ListView_ResizeEvent((S_ListView*)w->object, ev);
        break;
    case WIDGET_TYPE_LISTVIEWITEM:
        S_ListViewItem_ResizeEvent((S_ListViewItem*)w->object, ev);
        break;
    case WIDGET_TYPE_SCROLLBAR:
        Scroll_ResizeEvent((Scroll*)w->object, ev);
        break;
    case WIDGET_TYPE_SLIDER:
        Slider_ResizeEvent((Slider*)w->object, ev);
        break;
    default:
        break;
    }
}

void Widget_SetBackgroundColor(S_Widget *w, Color c){
    memcpy(&w->backgroundColor, &c, sizeof(Color));
}

void Widget_SetForegroundColor(S_Widget *w, Color c){
    memcpy(&w->foregroundColor, &c, sizeof(Color));
}

void Widget_SetBackgroundImage(S_Widget *w, Pixmap image){
    w->backgroundImage = image;
}

void Widget_SetDock(S_Widget *w, int dock){
    XSetWindowAttributes attr;
    switch(dock){
    case DOCK_RIGHT:
    case DOCK_RIGHT_STRACH:
        attr.win_gravity = EastGravity;
        break;
    case DOCK_RIGHT_TOP:
        attr.win_gravity = NorthEastGravity;
        break;
    case DOCK_RIGHT_BOTTOM:
        attr.win_gravity = SouthEastGravity;
        break;
    case DOCK_TOP:
    case DOCK_TOP_STRACH:
        attr.win_gravity = NorthGravity;
        break;
    case DOCK_LEFT:
    case DOCK_LEFT_STRACH:
        attr.win_gravity = WestGravity;
        break;
    case DOCK_LEFT_TOP:
        attr.win_gravity = NorthWestGravity;
        break;
    case DOCK_LEFT_BOTTOM:
        attr.win_gravity = SouthWestGravity;
        break;
    case DOCK_BOTTOM:
    case DOCK_BOTTOM_STRACH:
        attr.win_gravity = SouthGravity;
        break;
    default:
        attr.win_gravity = StaticGravity;//CenterGravity;
        break;
    }
    XChangeWindowAttributes(w->display, w->window, CWWinGravity, &attr);
    w->dock = dock;
}

void Widget_SetBorderColor(S_Widget *w, Color c){
    XSetWindowAttributes attr;
    attr.border_pixel = c.l;
    XChangeWindowAttributes(w->display, w->window, CWBorderPixel, &attr);
}

void Widget_SetBorderWidth(S_Widget *w, int width){
    XSetWindowBorderWidth(w->display, w->window, width);
}

void Widget_SetVisibility(S_Widget *w, char v){
    w->visibility = v;
}
