#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>

#include "listview.h"

void ListView_MoveScroll(void *ss, int val){
    Scroll *s = (Scroll*)ss;
    S_ListView *lv = (S_ListView*)s->sw->object;
    if(s->type == VERTICAL_SCROLL){
        if(val != 0){
            int y = lv->widget->y - val;
            int ty = lv->s->widget->y + val;
            if(y > 0) {y = 0; ty = 0;}
            //else if(y < lv->widget->parent->height - lv->widget->height) y = lv->widget->parent->height - lv->widget->height;
            Widget_Move(lv->widget, lv->widget->x, y);
            Widget_Move(lv->s->widget, lv->s->widget->x, ty);
            ListView_Paint(lv, NULL);
//            int i;
//            for(i = 0; i < lv->items_count; i++){
//                Widget_Move(lv->items[i]->widget, lv->items[i]->widget->x, lv->items[i]->widget->y - val);
//            }
        }
    }
}

S_ListView *ListView_newListView(S_Widget *parent, int x, int y, int width, int height){
    S_ListView *lv = (S_ListView*)malloc(sizeof(S_ListView));
    lv->s = Scroll_newScroll(parent, x, y, width, height, VERTICAL_SCROLL | HORIZONTAL_SCROLL);
    lv->widget = Widget_newWidget(0, 0, width, height, lv->s->widget);
    lv->widget->type = WIDGET_TYPE_LISTVIEW;
    lv->widget->object = (void*)lv;
    lv->items = NULL;
    lv->selectedItem = NULL;
    lv->items_count = 0;
    lv->focus = 0;

    XSelectInput(lv->widget->display, lv->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask);

    //lv->s = Scroll_newScroll(lv->widget, lv->widget->width - 15, 0, 15, lv->widget->height, VERTICAL_SCROLL, ListView_MoveScroll);
    return lv;
}

void ListView_Dispose(S_ListView *lv){
    if(!lv->widget->disposed && lv->items){
        int i;
        for(i = 0; i < lv->items_count; i++){
            Widget_RemoveChild(lv->widget, lv->items[i]->widget);
        }
        free(lv->items);
    }
    if(!lv->widget->disposed) Widget_Dispose(lv->widget);
    free(lv);
}

void ListView_AddItem(S_ListView *lv, S_ListViewItem *item){
        lv->items = (S_ListViewItem**)realloc(lv->items, sizeof(S_ListViewItem*)*(lv->items_count+1));
        lv->items[lv->items_count++] = item;
        if(lv->widget->height < lv->items_count * item->widget->height) Widget_Resize(lv->widget, lv->widget->width, lv->widget->height + item->widget->height);
        Widget_Move(item->widget, item->widget->x, (lv->items_count-1)*item->widget->height);
        ListView_Paint(lv, NULL);
//        if(lv->s->type == VERTICAL_SCROLL){
//            Scroll_SetSwSize(lv->s, lv->widget->height);
//        }
}

void ListView_RemoveItem(S_ListView *lv, S_ListViewItem *item){
    int i;
    for(i = 0; i < lv->items_count; i++){
        if(lv->items[i] == item){
            S_ListViewItem **tmp = NULL;
            if(lv->items_count - 1 > 0){
                tmp = (S_ListViewItem**)malloc(sizeof(S_ListViewItem*)*(lv->items_count - 1));
                memcpy(tmp, lv->items, sizeof(S_ListViewItem*)*i);
                memcpy(&tmp[i], &lv->items[i+1], sizeof(S_ListViewItem*)*(lv->items_count - (i+1)));
            }
            Widget_RemoveChild(lv->widget, item->widget);
            free(lv->items);
            lv->items = tmp;
            lv->items_count--;
            Widget_Resize(lv->widget, lv->widget->width, lv->widget->height - item->widget->height);
//            if(lv->s->type == VERTICAL_SCROLL){
//                Scroll_SetSwSize(lv->s, lv->widget->height);
//            }
            break;
        }
    }
}

void ListView_ItemSelectLink(S_ListView *lv, char *link){
    int i;
    for(i = 0; i < lv->items_count; i++){
        if(lv->items[i]->object == (void*)link){
            ListView_ItemSelect(lv, lv->items[i]);
//            if(lv->items[i]->widget->y > lv->s->widget->height){
//                int per = (lv->items[i]->widget->y * 100)/lv->widget->height;
//                Slider_MovePer(lv->s->slVer, -100);
//                Slider_MovePer(lv->s->slVer, per);
//            }
//            else {
//                Slider_MovePer(lv->s->slVer, -100);
//            }
        }
    }
}

void ListView_ItemSelect(S_ListView *lv, S_ListViewItem *item){
    if(lv->selectedItem) S_ListViewItem_Select(lv->selectedItem, 0);
    lv->selectedItem = item;
    S_ListViewItem_Select(item, 1);
    if(lv->s->slVer->widget->visible){
        int a = item->widget->y + lv->widget->y + item->widget->height;
        if(lv->s->slHor->widget->visible && a > item->widget->height) a += lv->s->slHor->widget->height;
        else if(a <= item->widget->height) a -= item->widget->height;
        if(a > lv->s->widget->height || a < 0){
            int per = (item->widget->y * 100)/lv->widget->height;
            //Slider_MovePer(lv->s->slVer, -100);
            Slider_MovePer(lv->s->slVer, per);
        }

//        else {
//            Slider_MovePer(lv->s->slVer, -100);
//        }
    }
}

void ListView_Paint(S_ListView *lv, PaintEventArgs *ev){
    Graphics *g = lv->widget->g;
    Color backcolor = Color_GetColor1(250, 250, 250, lv->widget->display);
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, backcolor);
    Graphics_FillRectangle(g, 0, 0, lv->widget->width, lv->widget->height);
    if(lv->items_count == 0){
        char *text = "(Список пуст)";
        int x = lv->widget->width/2 - XTextWidth(g->fontInfo, text, strlen(text))/2;
        int y = lv->widget->height/2 - 7;
        Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
        Graphics_DrawText(g, x, y, text);
    }
    Graphics_EndPaint(g);
}

void ListView_ResizeEvent(S_ListView *lv, ResizeEventArgs *ev){
    //Widget_Resize(lv->widget, ev->width, ev->height);
//    if(lv->s->type == VERTICAL_SCROLL){
//        Scroll_SetSize(lv->s, lv->s->widget->width, ev->height);
//    }
}

void ListView_KeyPress(S_ListView *lv, KeyEventArgs *ev){
    //char a = 0;
    int i;
    if(!lv->focus) return;
    if(ev->ksym != XK_Up && ev->ksym != XK_Down && ev->ksym != XK_Return) return;
//    if(lv->widget->window == ev->win) a = 1;
//    else{
//        for(i = 0; i < lv->items_count; i++){
//            if(lv->items[i]->widget->window == ev->win){
//                a = 1;
//                break;
//            }
//        }
//    }
   // if(a){
        switch(ev->ksym){
        case XK_Up:
            if(lv->items_count > 0 && lv->selectedItem == NULL) ListView_ItemSelect(lv, lv->items[0]);
            else{
                for(i = 0; i < lv->items_count; i++){
                    if(lv->items[i] == lv->selectedItem){
                        if(i == 0) break;
                        ListView_ItemSelect(lv, lv->items[i-1]);
                        break;
                    }
                }
            }
            break;
        case XK_Down:
            if(lv->items_count > 0 && lv->selectedItem == NULL) ListView_ItemSelect(lv, lv->items[0]);
            else{
                for(i = 0; i < lv->items_count; i++){
                    if(lv->items[i] == lv->selectedItem){
                        if(i+1 == lv->items_count) break;
                        ListView_ItemSelect(lv, lv->items[i+1]);
                        break;
                    }
                }
            }
            break;
        case XK_Return:
            if(lv->items_count > 0 && lv->selectedItem != NULL && lv->selectedItem->ItemSelect != NULL)
                lv->selectedItem->ItemSelect(lv->selectedItem->sender, lv->selectedItem->object);
            break;
        default: break;
        }
    //}
}

void ListView_Tab(S_ListView *lv){
    lv->focus = 1;
    if(!lv->selectedItem && lv->items_count > 0) ListView_ItemSelect(lv, lv->items[0]);
    if(lv->selectedItem) S_ListViewItem_Paint(lv->selectedItem, NULL);
}

void ListView_Untab(S_ListView *lv){
    lv->focus = 0;
    if(lv->selectedItem) S_ListViewItem_Paint(lv->selectedItem, NULL);
}

void ListView_SetEnd(Button *b, void *vlv){
    S_ListView *lv = (S_ListView*)vlv;
    if(lv->items_count > 0) ListView_ItemSelect(lv, lv->items[lv->items_count-1]);
}

void ListView_SetStart(Button *b, void *vlv){
    S_ListView *lv = (S_ListView*)vlv;
    if(lv->items_count > 0) ListView_ItemSelect(lv, lv->items[0]);
}

void ListView_SetNext(Button *b, void *vlv){
    S_ListView *lv = (S_ListView*)vlv;
    int i;
    if(lv->items_count > 0 && lv->selectedItem == NULL) ListView_ItemSelect(lv, lv->items[0]);
    else{
        for(i = 0; i < lv->items_count; i++){
            if(lv->items[i] == lv->selectedItem){
                if(i+1 == lv->items_count) break;
                ListView_ItemSelect(lv, lv->items[i+1]);
                break;
            }
        }
    }
}

void ListView_SetPrev(Button *b, void *vlv){
    S_ListView *lv = (S_ListView*)vlv;
    int i;
    if(lv->items_count > 0 && lv->selectedItem == NULL) ListView_ItemSelect(lv, lv->items[0]);
    else{
        for(i = 0; i < lv->items_count; i++){
            if(lv->items[i] == lv->selectedItem){
                if(i == 0) break;
                ListView_ItemSelect(lv, lv->items[i-1]);
                break;
            }
        }
    }
}
