#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <X11/xpm.h>
#include "widget.h"
#include "panel.h"

#define PANEL_BACKGROUND_COLOR Color_GetColor(245, 245, 245)

Panel *Panel_newPanel(int x, int y, int width, int height, S_Widget *parent){
    Panel *p = (Panel*)malloc(sizeof(Panel));
    if(p == NULL) return NULL;
    S_Widget *w = Widget_newWidget(x, y, width, height, parent);
    if(w == NULL){
        free(p);
        return NULL;
    }
    w->type = WIDGET_TYPE_PANEL;
    w->object = (void*)p;
    XSelectInput(w->display, w->window, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask |
                 EnterWindowMask | LeaveWindowMask);
    p->widget = w;
    p->backgroundImage = NULL;
    p->active = 0;
    p->hover = 0;
    p->id = 0;
    p->selected = 0;
    p->title = NULL;
    p->KeyDown = NULL;
    p->KeyUp = NULL;
    p->MouseMove = NULL;
    p->MousePress = NULL;
    p->MouseRelease = NULL;
    p->Paint = NULL;
    return p;
}

void Panel_Dispose(Panel *p){
    if(p->title != NULL) free(p->title);
    if(p->backgroundImage != NULL) XDestroyImage(p->backgroundImage);
    free(p);
    p = NULL;
}

void Panel_Show(Panel *p){
    Widget_Show(p->widget);
}

void Panel_Hide(Panel *p){
    Widget_Hide(p->widget);
}

void Panel_Move(Panel *p, int x, int y){
    Widget_Move(p->widget, x, y);
}

void Panel_Resize(Panel *p, int width, int height){
    Widget_Resize(p->widget, width, height);
}

void Panel_SetTitle(Panel *p, char *title){
    if(title == NULL) return;
    if(p->title != NULL) free(p->title);
    p->title = strdup(title);
    Panel_Paint(p, NULL);
}

void Panel_Paint(Panel *p, PaintEventArgs *ev){
    Graphics *g;
    int xt, yt;
    char *tcon;
    if(p->Paint != NULL){
        p->Paint(p, ev);
        return;
    }
    g = p->widget->g;
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, PANEL_BACKGROUND_COLOR);
    Graphics_FillRectangle(g, 0, 0, p->widget->width, p->widget->height);
    if(p->backgroundImage != NULL){
        XPutImage(p->widget->display, g->context, g->gc, p->backgroundImage, 0, 0, 0, 0, p->widget->width, p->widget->height);
    }
    else{
        Graphics_SetColor(g, Color_GetColor1(0, 0, 0, g->widget->display));
        tcon = "Соединение...";
        xt = XTextWidth(g->fontInfo, tcon, strlen(tcon));
        xt = (p->widget->width>>1)-(xt>>1);
        yt = (p->widget->height>>1)-7;
        Graphics_DrawText(g, xt, yt, tcon);
    }
    if(p->selected){
        Graphics_SetColor(g, Color_GetColor(0, 255, 0));
        Graphics_DrawRectangle(g, 0, 0, p->widget->width-1, p->widget->height-1);
    }
    Graphics_EndPaint(g);
}

void Panel_MouseEnter(Panel *p, MouseEventArgs *ev){
    p->hover = 1;
    p->selected = 1;
    Panel_Paint(p, NULL);
}

void Panel_MouseLeave(Panel *p, MouseEventArgs *ev){
    p->hover = 0;
    p->selected = 0;
    Panel_Paint(p, NULL);
}

void Panel_MousePress(Panel *p, MouseEventArgs *ev){
    if(p->MousePress != NULL){
        p->MousePress(p, ev);
    }
}

void Panel_MouseMove(Panel *p, MouseEventArgs *ev){
    if(p->MouseMove != NULL){
        p->MouseMove(p, ev);
    }
}

void Panel_MouseRelease(Panel *p, MouseEventArgs *ev){
    if(p->MouseRelease != NULL){
        p->MouseRelease(p, ev);
    }
}

void Panel_KeyDown(Panel *p, KeyEventArgs *ev){
    if(p->KeyDown != NULL){
        p->KeyDown(p, ev);
    }
}

void Panel_KeyUp(Panel *p, KeyEventArgs *ev){
    if(p->KeyUp != NULL){
        p->KeyUp(p, ev);
    }
}

void Panel_Tab(Panel *p){
    p->selected = 1;
    Panel_Paint(p, NULL);
}

void Panel_Untab(Panel *p){
    p->selected = 0;
    Panel_Paint(p, NULL);
}

void Panel_SetBackgroundImage(Panel *p, XImage *im){
    p->backgroundImage = im;
    Panel_Paint(p, NULL);
}
