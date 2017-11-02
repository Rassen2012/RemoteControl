#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "button.h"

#define BUTTON_BACKGROUND_COLOR(d) Color_GetColor1(225, 225, 225, d)
#define BUTTON_FOREGROUND_COLOR(d) Color_GetColor1(0, 0, 0, d)
#define BUTTON_BORDER_COLOR(d) Color_GetColor1(173, 173, 173, d)
#define BUTTON_FONT_COLOR(d) Color_GetColor1(0, 0, 0, d)
#define BUTTON_HOVER_BACKGROUND_COLOR(d) Color_GetColor1(229, 241, 251, d)
#define BUTTON_HOVER_BORDER_COLOR(d) Color_GetColor1(0, 120, 215, d)
#define BUTTON_DISABLED_BACKGROUND_COLOR(d) Color_GetColor1(204, 204, 204, d)
#define BUTTON_DISABLED_BORDER_COLOR(d) Color_GetColor1(191, 191, 191, d)
#define BUTTON_DISABLED_FONT_COLOR(d) Color_GetColor1(144, 144, 168, d)
#define BUTTON_PRESSED_BACKGROUND_COLOR(d) Color_GetColor1(204, 228, 247, d)
#define BUTTON_PRESSED_BORDER_COLOR(d) Color_GetColor1(0, 84, 153, d)
#define BUTTON_FONT_SIZE 14
#define BUTTON_BORDER_WIDTH 1
#define BUTTON_TAB_BORDER_WIDTH 2
#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 20

Button *Button_newButton(int x, int y, int width, int height, char *text, S_Widget *parent){
    Button *b = (Button*)malloc(sizeof(Button));
    if(b == NULL) return NULL;
    b->widget = Widget_newWidget(x, y, width, height, parent);
    if(b->widget == NULL){
        free(b);
        return NULL;
    }
    b->widget->object = (void*)b;
    b->widget->type = WIDGET_TYPE_BUTTON;
    XSelectInput(b->widget->display, b->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask);
    b->focus = 0;
    b->hover = 0;
    b->visible = 1;
    b->enabled = 1;
    b->pressed = 0;
    b->Paint = NULL;
    b->arg = NULL;
    b->Click = NULL;
    b->fontSize = 12;
    b->tab = 0;
    b->text = text;
    //Button_SetText(b, text);
    Button_SetBackgroundColor(b, BUTTON_BACKGROUND_COLOR(b->widget->display));
    Button_SetForegroundColor(b, BUTTON_FOREGROUND_COLOR(b->widget->display));
    Button_SetFontSize(b, BUTTON_FONT_SIZE);
    return b;
}

void Button_Dispose(Button *b){
    free(b);
}

void Button_Show(Button *b){
    Widget_Show(b->widget);
    b->visible = 1;
}

void Button_Hide(Button *b){
    b->visible = 0;
    Widget_Hide(b->widget);
}

void Button_SetBackgroundColor(Button *b, Color c){
    Widget_SetBackgroundColor(b->widget, c);
    Button_Paint(b, NULL);
}

void Button_SetForegroundColor(Button *b, Color c){
    Widget_SetForegroundColor(b->widget, c);
    Button_Paint(b, NULL);
}

void Button_SetBackgroundImage(Button *b, Pixmap image){
    Widget_SetBackgroundImage(b->widget, image);
    Button_Paint(b, NULL);
}

Color Button_GetBackgroundColor(Button *b){
    return b->widget->backgroundColor;
}

Color Button_GetForegroundColor(Button *b){
    return b->widget->foregroundColor;
}

Pixmap Button_GetBackgroundImage(Button *b){
    return b->widget->backgroundImage;
}

void Button_Enable(Button *b){
    b->enabled = 1;
    Button_Paint(b, NULL);
}

void Button_Disable(Button *b){
    b->enabled = 0;
    Button_Paint(b, NULL);
}

void Button_SetTab(Button *b){
    if(b->visible && b->enabled){
        b->tab = 1;
        b->focus = 1;
        Button_Paint(b, NULL);
    }
    else{
        Widget_Tab(b->widget);
    }
}

void Button_Untab(Button *b){
    b->tab = 0;
    b->focus = 0;
    Button_Paint(b, NULL);
}

void Button_Move(Button *b, int x, int y){
    Widget_Move(b->widget, x, y);
}

void Button_Resize(Button *b, int width, int height){
    Widget_Resize(b->widget, width, height);
    Button_Paint(b, NULL);
}

void Button_SetText(Button *b, char *text){
    b->text = strdup(text);
    Button_Paint(b, NULL);
}

void Button_KeyPress(Button *b, KeyEventArgs *ev){
    if(b->focus && ev->keyCode == KEY_ENTER){
        b->pressed = 1;
        Button_Paint(b, NULL);
    }
}

void Button_KeyRelease(Button *b, KeyEventArgs *ev){
    if(b->focus && ev->keyCode == KEY_ENTER){
        b->pressed = 0;
        Button_Paint(b, NULL);
        if(b->Click != NULL){
            b->Click(b, b->arg);
        }
    }
}

void Button_MouseEnter(Button *b, MouseEventArgs *ev){
    if(ev->win == b->widget->window){
        b->hover = 1;
        Button_Paint(b, NULL);
    }
}

void Button_MouseLeave(Button *b, MouseEventArgs *ev){
    if(ev->win == b->widget->window){
        b->hover = 0;
        Button_Paint(b, NULL);
    }
}

void Button_MousePress(Button *b, MouseEventArgs *ev){
    if(ev->win == b->widget->window){
        b->pressed = 1;
        Button_Paint(b, NULL);
    }
}

void Button_MouseRelease(Button *b, MouseEventArgs *ev){
    if(ev->win == b->widget->window && b->pressed){
        b->pressed = 0;
        Button_Paint(b, NULL);
        if(b->Click != NULL){
            b->Click(b, b->arg);
        }
    }
}

void Button_MouseMove(Button *b, MouseEventArgs *ev){

}

void Button_ResizeEvent(Button *b, ResizeEventArgs *ev){
    int width = b->widget->width;
    int height = b->widget->height;
    if(ev->width != ev->old_width || ev->height != ev->old_height){
        switch(b->widget->dock){
        case DOCK_STRACH:
            width += ev->width - ev->old_width;
            height += ev->height - ev->old_height;
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
        Button_Resize(b, width, height);
    }
}

void Button_Paint(Button *b, PaintEventArgs *ev){
    Color back, fore, border;
    int borderwidth;
    int xt, yt;
    Graphics *g;
    if(!b->visible) return;
    if(ev != NULL && ev->win != b->widget->window) return;
    if(b->Paint != NULL){
        b->Paint(b, ev);
        return;
    }
    if(!b->enabled){
        back = BUTTON_DISABLED_BACKGROUND_COLOR(b->widget->display);
        fore = BUTTON_DISABLED_FONT_COLOR(b->widget->display);
        border = BUTTON_DISABLED_BORDER_COLOR(b->widget->display);
        borderwidth = BUTTON_BORDER_WIDTH;
    }
    else if(b->pressed){
        back = BUTTON_PRESSED_BACKGROUND_COLOR(b->widget->display);
        fore = BUTTON_FONT_COLOR(b->widget->display);
        border = BUTTON_PRESSED_BORDER_COLOR(b->widget->display);
        borderwidth = BUTTON_BORDER_WIDTH;
    }
    else if(b->hover){
        back = BUTTON_HOVER_BACKGROUND_COLOR(b->widget->display);
        fore = BUTTON_FONT_COLOR(b->widget->display);
        border = BUTTON_HOVER_BORDER_COLOR(b->widget->display);
        borderwidth = BUTTON_BORDER_WIDTH;
    }
    else if(b->tab){
        back = BUTTON_BACKGROUND_COLOR(b->widget->display);
        fore = BUTTON_FONT_COLOR(b->widget->display);
        border = BUTTON_HOVER_BORDER_COLOR(b->widget->display);
        borderwidth = BUTTON_TAB_BORDER_WIDTH;
    }
    else{
        back = BUTTON_BACKGROUND_COLOR(b->widget->display);
        fore = BUTTON_FONT_COLOR(b->widget->display);
        border = BUTTON_BORDER_COLOR(b->widget->display);
        borderwidth = BUTTON_BORDER_WIDTH;
    }
    g = b->widget->g;
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, back);
    Graphics_FillRectangle(g, 0, 0, b->widget->width, b->widget->height);
    if(b->widget->backgroundImage != 0){
        Graphics_DrawImage(g, b->widget->backgroundImage, 0, 0, b->widget->width, b->widget->height, 0, 0);
    }
    if(b->text){
        Graphics_SetColor(g, fore);
        Graphics_SetFontSize(g, b->fontSize);
        xt = XTextWidth(g->fontInfo, b->text, strlen(b->text));
        xt = (b->widget->width>>1) - (xt>>1);
        yt = (b->widget->height>>1)+(b->fontSize>>1) - 2;
        Graphics_DrawText(g, xt, yt, b->text);
    }
    Graphics_SetColor(g, border);
    Graphics_SetLineWidth(g, borderwidth);
    Graphics_DrawRectangle(g, borderwidth-1, borderwidth-1, b->widget->width-borderwidth, b->widget->height-borderwidth);
    Graphics_EndPaint(g);
}

void Button_SetFontSize(Button *b, int size){
    b->fontSize = size;
}

void Button_SetDock(Button *b, int dock){
    Widget_SetDock(b->widget, dock);
}
