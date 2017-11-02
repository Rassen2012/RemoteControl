#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include "widget.h"
#include "label.h"

#define LABEL_BACKGROUND_COLOR Color_GetColor(255, 255, 255)
#define LABEL_FOREGROUND_COLOR Color_GetColor(0, 0, 0)

Label *Label_newLabel(int x, int y, char *text, S_Widget *parent){
    Label *lbl = (Label*)malloc(sizeof(Label));
    int tw;
    if(lbl == NULL) return NULL;
    lbl->w = Widget_newWidget(x, y, 2, 2, parent);
    if(lbl->w == NULL){
        free(lbl);
        return NULL;
    }
    lbl->w->type = WIDGET_TYPE_LABEL;
    lbl->w->object = (void*)lbl;
    lbl->w->is_tabed = 0;
    XSelectInput(lbl->w->display, lbl->w->window, ExposureMask);
    Label_SetFontSize(lbl, 12);
    if(text != NULL){
        lbl->text = (char*)malloc(sizeof(char)*(strlen(text)+1));
        //snprintf(lbl->text, sizeof(char)*(strlen(text)+1), "%s\0", text);
        strncpy(lbl->text, text, strlen(text));
        lbl->text[strlen(text)] = 0;
        tw = XTextWidth(lbl->w->g->fontInfo, lbl->text, strlen(lbl->text));
        Graphics_SetFontSize(lbl->w->g, lbl->fontSize);
        Widget_Resize(lbl->w, tw, lbl->fontSize);
    }
    else{
        lbl->text = NULL;
    }
    lbl->Paint = NULL;
    lbl->visible = 0;
    Label_SetBackgroundColor(lbl, LABEL_BACKGROUND_COLOR);
    Label_SetForegroundColor(lbl, LABEL_FOREGROUND_COLOR);
    return lbl;
}

void Label_Dispose(Label *lbl){
    if(lbl == NULL) return;
    //Widget_Dispose(lbl->w);
    if(lbl->text != NULL){
        free(lbl->text);
    }
    free(lbl);
}

void Label_SetFontSize(Label *lbl, int size){
    lbl->fontSize = size;
    Label_Resize(lbl, lbl->w->width, size);
    Graphics_SetFontSize(lbl->w->g, lbl->fontSize);
    Label_Paint(lbl, NULL);
}

void Label_SetText(Label *lbl, char *text){
    int tw;
    if(text == NULL || strlen(text) == 0) return;
    if(lbl->text != NULL){
        free(lbl->text);
    }
    lbl->text = (char*)malloc(sizeof(char)*(strlen(text)+1));
    //snprintf(lbl->text, sizeof(char)*(strlen(text)+1), "%s\0", text);
    strncpy(lbl->text, text, strlen(text));
    lbl->text[strlen(text)] = 0;
    tw = XTextWidth(lbl->w->g->fontInfo, lbl->text, strlen(lbl->text));
    Label_Resize(lbl, tw, lbl->w->height);
    Label_Paint(lbl, NULL);
}

void Label_Show(Label *lbl){
    Widget_Show(lbl->w);
    lbl->visible = 1;
}

void Label_Hide(Label *lbl){
    Widget_Hide(lbl->w);
    lbl->visible = 0;
}

void Label_Move(Label *lbl, int x, int y){
    Widget_Move(lbl->w, x, y);
}

void Label_Resize(Label *lbl, int width, int height){
    Widget_Resize(lbl->w, width, height);
}

void Label_SetBackgroundColor(Label *lbl, Color c){
    lbl->w->backgroundColor = c;
    Label_Paint(lbl, NULL);
}

void Label_SetForegroundColor(Label *lbl, Color c){
    lbl->w->foregroundColor = c;
    Label_Paint(lbl, NULL);
}

void Label_SetBackgroundImage(Label *lbl, Pixmap pic){
    lbl->w->backgroundImage = pic;
    Label_Paint(lbl, NULL);
}

Pixmap Label_GetBackgroundImage(Label *lbl){
    return lbl->w->backgroundImage;
}

Color Label_GetBackgroundColor(Label *lbl){
    return lbl->w->backgroundColor;
}

Color Label_GetForegroundColor(Label *lbl){
    return lbl->w->foregroundColor;
}

int Label_X(Label *lbl){
    return lbl->w->x;
}

int Label_Y(Label *lbl){
    return lbl->w->y;
}

int Label_Width(Label *lbl){
    return lbl->w->width;
}

int Label_Height(Label *lbl){
    return lbl->w->height;
}

void Label_Paint(Label *lbl, PaintEventArgs *ev){
    Graphics *g;
    if(!lbl->visible) return;
    if(ev != NULL && lbl->w->window != ev->win) return;
    g = lbl->w->g;
    Graphics_BeginPaint(g, ev);
    Graphics_SetColor(g, lbl->w->backgroundColor);
    Graphics_FillRectangle(g, 0, 0, lbl->w->width, lbl->w->height);
    if(lbl->text != NULL){
        Graphics_SetFontSize(g, lbl->fontSize);
        Graphics_SetColor(g, lbl->w->foregroundColor);
        Graphics_DrawText(g, 0, lbl->w->height, lbl->text);
    }
    Graphics_EndPaint(g);
}
