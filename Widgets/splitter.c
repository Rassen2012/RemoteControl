#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <X11/cursorfont.h>
#include "splitter.h"

#define SPACE_WIDTH 4

void Splitter_CreateSplittCursor(Splitter *sp);
void Splitter_CreateNormalCursor(Splitter *sp);

Splitter *Splitter_newSplitter(int x, int y, int width, int height, S_Widget *parent, S_Widget *leftWidget, S_Widget *rightWidget){
    Splitter *sp = (Splitter*)malloc(sizeof(Splitter));
    if(sp == NULL) return NULL;
    sp->oldX = sp->oldY = sp->spaceHover = sp->spacePressed = 0;
    sp->mainWidget = Widget_newWidget(x, y, width, height, parent);
    if(sp->mainWidget == NULL){
        free(sp);
        return NULL;
    }
    sp->mainWidget->is_tabed = 0;
    sp->leftWindow = NULL;
    if(leftWidget) sp->leftWindow = leftWidget;
    else{
        sp->leftWindow = Widget_newWidget(0, 0, (width>>1)-(SPACE_WIDTH>>1), height, sp->mainWidget);
        sp->leftWindow->type = -1;
        if(sp->leftWindow == NULL){
            Widget_Dispose(sp->mainWidget);
            free(sp);
            return NULL;
        }
    }
    if(rightWidget) sp->rightWindow = rightWidget;
    else{
        sp->rightWindow = Widget_newWidget((width>>1)+SPACE_WIDTH, 0, sp->leftWindow->width, height, sp->mainWidget);
        sp->rightWindow->type = -1;
        if(sp->rightWindow == NULL){
            Widget_Dispose(sp->leftWindow);
            Widget_Dispose(sp->mainWidget);
            free(sp);
            return NULL;
        }
    }
    //sp->leftWindow = leftWidget;
    //sp->rightWindow = rightWidget;
    //if(sp->leftWindow) Widget_AddChild(sp->mainWidget, sp->leftWindow);
    //if(sp->rightWindow) Widget_AddChild(sp->mainWidget, sp->rightWindow);
    sp->mainWidget->type = WIDGET_TYPE_SPLITTER;
    sp->mainWidget->object = (void*)sp;
    sp->left_width = 50.0;
    sp->right_width = 50.0;

    XSelectInput(sp->mainWidget->display, sp->mainWidget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);

    Widget_SetBorderWidth(sp->mainWidget, 1);
    return sp;
}

void Splitter_Dispose(Splitter *sp){
    //Widget_Dispose(sp->mainWidget);
    free(sp);
}

void Splitter_SetWidgets(Splitter *sp, S_Widget *leftWidget, S_Widget *rightWidget){
//    if(sp->leftWindow) Widget_Dispose(sp->leftWindow);
//    if(sp->rightWindow) Widget_Dispose(sp->rightWindow);
//    sp->leftWindow = leftWidget;
//    sp->rightWindow = rightWidget;
//    Widget_AddChild(sp->mainWidget, sp->leftWindow);
//    Widget_AddChild(sp->mainWidget, sp->rightWindow);
    Splitter_SetLeftWidget(sp, leftWidget);
    Splitter_SetRightWidget(sp, rightWidget);
}

void Splitter_SetLeftWidget(Splitter *sp, S_Widget *leftWidget){
    //if(sp->leftWindow) Widget_Dispose(sp->leftWindow);
    //sp->leftWindow = leftWidget;
    Widget_RemoveChild(sp->mainWidget, sp->leftWindow);
    //Widget_AddChild(sp->mainWidget, leftWidget);
    sp->leftWindow = leftWidget;
    //Splitter_SetSize(sp, 30);
}

void Splitter_SetRightWidget(Splitter *sp, S_Widget *rightWidget){
    //if(sp->rightWindow) Widget_Dispose(sp->rightWindow);
    //sp->rightWindow = rightWidget;
    Widget_RemoveChild(sp->mainWidget, sp->rightWindow);
    sp->rightWindow = rightWidget;
    //Widget_AddChild(sp->rightWindow, rightWidget);
}

void Splitter_Paint(Splitter *sp, PaintEventArgs *ev){
    Graphics *g = sp->mainWidget->g;
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, Color_GetColor1(0, 0, 0, g->widget->display));
    Graphics_FillRectangle(g, 0, 0, sp->mainWidget->width, sp->mainWidget->height);
    Graphics_EndPaint(g);
}

void Splitter_MouseEnter(Splitter *sp, MouseEventArgs *ev){
    if(ev->x > sp->leftWindow->width && ev->x < sp->rightWindow->x) {
        if(!sp->spaceHover){
            sp->spaceHover = 1;
            Splitter_CreateSplittCursor(sp);
        }
    }
}

void Splitter_MouseLeave(Splitter *sp, MouseEventArgs *ev){
    if(!sp->spacePressed && sp->spaceHover){
        sp->spaceHover = 0;
        Splitter_CreateNormalCursor(sp);
    }
}

void Splitter_MousePress(Splitter *sp, MouseEventArgs *ev){
    if(sp->spaceHover) sp->spacePressed = 1;
}

void Splitter_MouseRelease(Splitter *sp, MouseEventArgs *ev){
    if(sp->spacePressed) sp->spacePressed = 0;
}

void Splitter_MouseMove(Splitter *sp, MouseEventArgs *ev){
    if(sp->spacePressed){
        if(sp->oldX != 0 && sp->oldY != 0){
            int width = sp->mainWidget->width;
            int left_width = sp->leftWindow->width + (ev->x - sp->oldX);
            int right_width = sp->rightWindow->width - (ev->x - sp->oldX);
            if(left_width <= 0 || right_width <= 0) return;
            sp->left_width = ((double)left_width * 100.0) / (double)(width - SPACE_WIDTH);
            sp->right_width = ((double)right_width * 100.0) / (double)(width - SPACE_WIDTH);
            sp->right_width += 1;
            //Widget_Resize(sp->leftWindow, left_width, sp->leftWindow->height);
            ResizeEventArgs rev;
            rev.old_height = sp->leftWindow->height;
            rev.old_width = sp->leftWindow->width;
            rev.width = left_width;
            rev.height = sp->leftWindow->height;
            rev.con = 1;
            rev.win = sp->leftWindow->window;
            Widget_ResizeEvent(sp->leftWindow, &rev);
            Widget_Move(sp->rightWindow, left_width + SPACE_WIDTH, sp->rightWindow->y);
            Widget_Resize(sp->rightWindow, right_width, sp->rightWindow->height);
        }
        sp->oldX = ev->x;
        sp->oldY = ev->y;
        return;
    }
    else if(sp->spaceHover){
        sp->oldX = ev->x;
        sp->oldY = ev->y;
    }
    else sp->oldX = sp->oldY = 0;
    if(ev->x > sp->leftWindow->width && ev->x < sp->rightWindow->x) {
        if(!sp->spaceHover) {
            sp->spaceHover = 1;
            Splitter_CreateSplittCursor(sp);
        }
    }
    else{
        if(sp->spaceHover){
            sp->spaceHover = 0;
            Splitter_CreateNormalCursor(sp);
        }
    }
}

void Splitter_ResizeEvent(Splitter *sp, ResizeEventArgs *ev){
    int width = ev->width;
    int height = ev->height;
    switch(sp->mainWidget->dock){
    case DOCK_STRACH:
        width = sp->mainWidget->parent->width - 10;//+= ev->width - ev->old_width;
        height = sp->mainWidget->parent->height - 10; //+= ev->height - ev->old_height;
        break;
    case DOCK_TOP_STRACH:
    case DOCK_BOTTOM_STRACH:
        width += ev->width - ev->old_width;
        break;
    case DOCK_LEFT_STRACH:
    case DOCK_RIGHT_STRACH:
        height += ev->height - ev->old_height;
        break;
    }
    ResizeEventArgs rev;
    Widget_Resize(sp->mainWidget, width, height);
    int left_width = (sp->left_width * (double)width) / 100.0 - SPACE_WIDTH/2;
    int right_width = (sp->right_width * (double)width) / 100.0 - SPACE_WIDTH/2;
    rev.old_width = sp->leftWindow->width;
    rev.old_height = sp->leftWindow->height;
    rev.width = left_width;
    rev.height = height - 5;
    rev.con = 1;
    rev.win = sp->leftWindow->window;
    //Widget_Resize(sp->leftWindow, left_width, height);
    Widget_ResizeEvent(sp->leftWindow, &rev);
    Widget_Move(sp->rightWindow, left_width + SPACE_WIDTH, sp->rightWindow->y);
//    rev.old_width = sp->rightWindow->width;
//    rev.old_height = sp->rightWindow->height;
//    rev.width = right_width;
//    rev.height = height;
//    rev.con = 1;
//    rev.win = sp->rightWindow->window;
    //Widget_ResizeEvent(sp->rightWindow, &rev);
    Widget_Resize(sp->rightWindow, right_width, height);
}

void Splitter_CreateSplittCursor(Splitter *sp){
    sp->cur = XCreateFontCursor(sp->mainWidget->display, XC_sb_h_double_arrow);
    XDefineCursor(sp->mainWidget->display, sp->mainWidget->window, sp->cur);
}

void Splitter_CreateNormalCursor(Splitter *sp){
    if(sp->cur != 0){
        XUndefineCursor(sp->mainWidget->display, sp->mainWidget->window);
        sp->cur = 0;
    }
}

void Splitter_SetSize(Splitter *sp, int leftPercent){
    sp->left_width = leftPercent;
    sp->right_width = 100 - leftPercent;
    int width = sp->mainWidget->width;
    int height = sp->mainWidget->height;
    int left_width = (sp->left_width * (double)width) / 100.0 - SPACE_WIDTH/2;
    int right_width = (sp->right_width * (double)width) / 100.0 - SPACE_WIDTH/2;
    ResizeEventArgs rev;
    rev.width = left_width;
    rev.height = height;
    rev.win = sp->leftWindow->window;
    Widget_ResizeEvent(sp->leftWindow, &rev);
    //Widget_Resize(sp->leftWindow, left_width, height);
    Widget_Move(sp->rightWindow, left_width + SPACE_WIDTH, sp->rightWindow->y);
    Widget_Resize(sp->rightWindow, right_width, height);
}
