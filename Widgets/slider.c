#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scrollbar.h"
#include "slider.h"

#define DEF_MINSIZE 15
#define DEF_STEP 10
#define DEF_SLIDSIZE 10
#define DEF_SLIDX 3
#define DEF_MINY DEF_MINSIZE
#define DEF_EMPSIZE (DEF_MINSIZE<<1)
#define DEF_MINX DEF_MINSIZE

void Slider_MovePer(Slider *sl, int per){
    int val;
    /*if(per > 0)*/ val = ((sl->widget->height - DEF_EMPSIZE/* - sl->size*/) * per) / 100;
    //else val = ((sl->widget->height - DEF_EMPSIZE - sl->size) * per) / 100;
    Slider_Move(sl, val);
}

void Slider_Move(Slider *sl, int val){
    if(sl->type == SLIDER_TYPE_VSCROLL){
        sl->slid_y += val;
        if(sl->slid_y <= DEF_MINY){
            if(!sl->max_up) sl->max_up = 1;
            if(sl->max_down) sl->max_down = 0;
            sl->slid_y = DEF_MINY;
        }
        else if(sl->slid_y > sl->widget->height - DEF_MINY - sl->size){
            if(!sl->max_down) sl->max_down = 1;
            if(sl->max_up) sl->max_up = 0;
            sl->slid_y = sl->widget->height - DEF_MINY - sl->size;
        }
        else{
            if(sl->max_down) sl->max_down = 0;
            if(sl->max_up) sl->max_up = 0;
        }
        Slider_Paint(sl, NULL);
        float all_per = sl->widget->height - DEF_EMPSIZE - sl->size;
        float cur_pos = sl->slid_y - DEF_MINSIZE;
        float tmpy = (cur_pos * 100.0f)/all_per;
        if(sl->mov != NULL) sl->mov((void*)sl, tmpy);
    }
    else if(sl->type == SLIDER_TYPE_HSCROLL){
        sl->slid_x += val;
        if(sl->slid_x <= DEF_MINX + sl->size){
            if(!sl->max_up) sl->max_up = 1;
            if(sl->max_down) sl->max_down = 0;
            sl->slid_x = DEF_MINX + sl->size;
        }
        else if(sl->slid_x > sl->widget->width - DEF_MINSIZE){
            if(!sl->max_down) sl->max_down = 1;
            if(sl->max_up) sl->max_up = 0;
            sl->slid_x = sl->widget->width - DEF_MINSIZE;
        }
        else{
            if(sl->max_down) sl->max_down = 0;
            if(sl->max_up) sl->max_up = 0;
        }
        Slider_Paint(sl, NULL);
        float all_per = sl->widget->width - DEF_EMPSIZE - sl->size;
        float cur_pos = sl->slid_x - DEF_MINSIZE - sl->size;
        float tmpx = (cur_pos * 100.0f)/all_per;
        if(sl->mov != NULL) sl->mov((void*)sl, tmpx);
    }
}

Slider *Slider_newSlider(S_Widget *parent, SLIDER_TYPE type, int x, int y, int width, int height, Slider_Mov m){
    Slider *sl = (Slider*)malloc(sizeof(Slider));
    sl->type = type;
    sl->min_size = DEF_MINSIZE;
    sl->mov = m;
    sl->hover = 0;
    sl->pressed = 0;
    sl->max_up = 1;
    sl->max_down = 0;
    sl->slid_x = sl->slid_y = 0;
    sl->slid_y = DEF_MINSIZE;
    sl->step = DEF_STEP;
    sl->widget = Widget_newWidget(x, y, width, height, parent);
    sl->widget->is_tabed = 0;
    sl->widget->type = WIDGET_TYPE_SLIDER;
    sl->widget->object = (void*)sl;

    XSelectInput(sl->widget->display, sl->widget->window, ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask |
                 ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask);

    if(sl->type == SLIDER_TYPE_VSCROLL){
        sl->size = height - (DEF_MINSIZE<<1);
    }
    else if(sl->type == SLIDER_TYPE_HSCROLL){
        sl->size = width - (DEF_MINSIZE<<1);
    }
    XRaiseWindow(sl->widget->display, sl->widget->window);
    return sl;
}

void Slider_Dispose(Slider *sl){
    if(!sl->widget->disposed) Widget_Dispose(sl->widget);
    free(sl);
}

void Slider_Paint(Slider *sl, PaintEventArgs *ev){
    Graphics *g = sl->widget->g;
    Display *d = sl->widget->display;
    Graphics_BeginPaint(g, ev);
    Color backcolor;
    int val = 210;
    Graphics_SetLineWidth(g, 1);
    int i;
    if(sl->type == SLIDER_TYPE_HSCROLL){
        for(i = 0; i < 15; i++, val += 3){
            backcolor = Color_GetColor1(val, val, val, d);
            Graphics_SetColor(g, backcolor);
            Graphics_DrawLine(g, 0, i, sl->widget->width, i);
        }
        backcolor = Color_GetColor1(210, 210, 210, d);
        Graphics_SetColor(g, backcolor);
        Graphics_FillRectangle(g, 0, 0, DEF_MINSIZE, DEF_MINSIZE);
        Graphics_FillRectangle(g, g->widget->width - DEF_MINSIZE, 0, DEF_MINSIZE, DEF_MINSIZE);
        if(sl->max_up) backcolor = Color_GetColor1(140, 140, 140, d);
        else backcolor = Color_GetColor1(10, 10, 10, d);
        Graphics_SetColor(g, backcolor);
        XPoint p1[3] = { {2, 7}, {10, 3}, {10, 11} };
        XPoint p2[3] = { {sl->widget->width - 10, 3}, {sl->widget->width - 2, 7}, {sl->widget->width - 10, 11} };
        Graphics_FillPolygon(g, p1, 3);
        if(sl->max_down) backcolor = Color_GetColor1(140, 140, 140, d);
        else backcolor = Color_GetColor1(10, 10, 10, d);
        Graphics_SetColor(g, backcolor);
        Graphics_FillPolygon(g, p2, 3);
        backcolor = Color_GetColor1(150, 150, 150, d);
        Graphics_SetColor(g, backcolor);
        Graphics_DrawRectangle(g, (sl->slid_x - sl->size) + 5, 3, sl->size - DEF_SLIDSIZE, DEF_SLIDSIZE);
        Graphics_SetLineWidth(g, 2);
        Graphics_DrawEllips(g, (sl->slid_x - sl->size) + 5, 8, 10, 10);
        Graphics_DrawEllips(g, sl->slid_x - 5, 8, 10, 10);
        Graphics_SetLineWidth(g, 1);
        if(sl->hover || sl->pressed) backcolor = Color_GetColor1(190, 190, 190, d);
        else backcolor = Color_GetColor1(210, 210, 210, d);
        Graphics_SetColor(g, backcolor);
        Graphics_FillRectangle(g, (sl->slid_x - sl->size) + 5, 4, sl->size - DEF_SLIDSIZE, DEF_SLIDSIZE-1);
        Graphics_FillEllips(g, (sl->slid_x - sl->size) + 5, 8, 10, 10);
        Graphics_FillEllips(g, sl->slid_x - 5, 8, 10, 10);
    }
    else if(sl->type == SLIDER_TYPE_VSCROLL){
        for(i = 0; i < 15; i++, val+=3){
            backcolor = Color_GetColor1(val, val, val, d);
            Graphics_SetColor(g, backcolor);
            Graphics_DrawLine(g, i, 0, i, sl->widget->height);
        }
        backcolor = Color_GetColor1(210, 210, 210, d);
        Graphics_SetColor(g, backcolor);
        Graphics_FillRectangle(g, 0, 0, DEF_MINSIZE, DEF_MINSIZE);
        Graphics_FillRectangle(g, 0, sl->widget->height-DEF_MINSIZE, DEF_MINSIZE, DEF_MINSIZE);
        if(sl->max_up) backcolor = Color_GetColor1(140, 140, 140, d);
        else backcolor = Color_GetColor1(10, 10, 10, d);
        Graphics_SetColor(g, backcolor);
        XPoint p[3] = { {3, 10}, {7, 2}, {11, 10}};
        Graphics_FillPolygon(g, p, 3);
        if(sl->max_down) backcolor = Color_GetColor1(140, 140, 140, d);
        else backcolor = Color_GetColor1(10, 10, 10, d);
        Graphics_SetColor(g, backcolor);
        XPoint xp[3] = {{3, sl->widget->height - 10}, {7, sl->widget->height - 2}, {11, sl->widget->height-10}};
        Graphics_FillPolygon(g, xp, 3);
        backcolor = Color_GetColor1(150, 150, 150, d);
        Graphics_SetColor(g, backcolor);
        Graphics_DrawRectangle(g, 3, sl->slid_y+5, DEF_SLIDSIZE, sl->size-10);
        Graphics_SetLineWidth(g, 2);
        Graphics_DrawEllips(g, 8, sl->slid_y+5, 10, 10);
        Graphics_DrawEllips(g, 8, (sl->slid_y+sl->size) - 5, 10, 10);
        Graphics_SetLineWidth(g, 1);

        if(sl->hover || sl->pressed) backcolor = Color_GetColor1(190, 190, 190, d);
        else backcolor = Color_GetColor1(210, 210, 210, d);

        Graphics_SetColor(g, backcolor);
        Graphics_FillRectangle(g, 4, sl->slid_y+5, DEF_SLIDSIZE-1, sl->size - 10);
        Graphics_FillEllips(g, 8, sl->slid_y+5, 10, 10);
        Graphics_FillEllips(g, 8, (sl->slid_y+sl->size) - 5, 10, 10);
    }

    Graphics_EndPaint(g);
}

void Slider_MouseDown(Slider *sl, MouseEventArgs *ev){
    if(sl->hover && ev->button == MOUSE_BUTTON_LEFT){
        sl->pressed = 1;
        sl->prev_x = ev->global_x;
        sl->prev_y = ev->global_y;
        Slider_Paint(sl, NULL);
    }
    else if(!sl->hover && ev->button == MOUSE_BUTTON_LEFT){
        if(sl->type == SLIDER_TYPE_HSCROLL){
            if(ev->x >= 0 && ev->x <= DEF_MINSIZE && !sl->max_up){
                Slider_Move(sl, -DEF_STEP);
            }
            else if(ev->x >= sl->widget->width - DEF_MINSIZE && ev->x <= sl->widget->width){
                Slider_Move(sl, DEF_STEP);
            }
            else{
                if(ev->x < sl->slid_x) Slider_Move(sl, ev->x - (sl->slid_x - sl->size));
                else Slider_Move(sl, ev->x - sl->slid_x);
            }
        }
        else if(sl->type == SLIDER_TYPE_VSCROLL){
            if(ev->y >= 0 && ev->y <= DEF_MINSIZE){
                Slider_Move(sl, -DEF_STEP);
            }
            else if(ev->y >= sl->widget->height - DEF_MINSIZE && ev->y <= sl->widget->height){
                Slider_Move(sl, DEF_STEP);
            }
            else{
                if(ev->y < sl->slid_y) Slider_Move(sl, ev->y - sl->slid_y);
                else Slider_Move(sl, ev->y - (sl->slid_y + sl->size));
            }
        }
    }
}

void Slider_MouseUp(Slider *sl, MouseEventArgs *ev){
    if(ev->button == MOUSE_BUTTON_LEFT && sl->pressed){
        sl->pressed = 0;
        Slider_Paint(sl, NULL);
    }
}

void Slider_MouseLeave(Slider *sl, MouseEventArgs *ev){
    if(sl->hover){
        sl->hover = 0;
        Slider_Paint(sl, NULL);
    }
}

void Slider_MouseMove(Slider *sl, MouseEventArgs *ev){
    if(sl->type == SLIDER_TYPE_VSCROLL){
        if(ev->x >= DEF_SLIDX && ev->x <= sl->widget->width-DEF_SLIDX && ev->y >= sl->slid_y && ev->y <= sl->slid_y + sl->size){
            if(!sl->hover){
                sl->hover = 1;
                Slider_Paint(sl, NULL);
            }
        }
        else {
            if(sl->hover){
                sl->hover = 0;
                Slider_Paint(sl, NULL);
            }
        }
        if(sl->pressed){
            Slider_Move(sl, ev->global_y - sl->prev_y);
            sl->prev_x = ev->global_x;
            sl->prev_y = ev->global_y;
        }
    }
    else if(sl->type == SLIDER_TYPE_HSCROLL){
        if(ev->x >= sl->slid_x - sl->size && ev->x <= sl->slid_x && ev->y >= DEF_SLIDX && ev->y <= sl->widget->height - DEF_SLIDX){
            if(!sl->hover){
                sl->hover = 1;
                Slider_Paint(sl, NULL);
            }
        }
        else {
            if(sl->hover){
                sl->hover = 0;
                Slider_Paint(sl, NULL);
            }
        }
        if(sl->pressed){
            Slider_Move(sl, ev->global_x - sl->prev_x);
            sl->prev_x = ev->global_x;
            sl->prev_y = ev->global_y;
        }
    }
}

void Slider_ResizeEvent(Slider *sl, ResizeEventArgs *ev){
    if(sl->type == SLIDER_TYPE_VSCROLL){
        Widget_Resize(sl->widget, sl->widget->width, sl->widget->height + (ev->height - ev->old_height));
        Widget_Move(sl->widget, ev->width - DEF_MINSIZE, sl->widget->y);
        //XRaiseWindow(sl->widget->display, sl->widget->window);
    }
    else if(sl->type == SLIDER_TYPE_HSCROLL){
        Widget_Resize(sl->widget, sl->widget->width + (ev->width - ev->old_width), sl->widget->height);
        Widget_Move(sl->widget, sl->widget->x, ev->height - DEF_MINSIZE);
    }
    Scroll_SetWidget((Scroll*)sl->widget->parent->object, ((Scroll*)sl->widget->parent->object)->sw);
    XRaiseWindow(sl->widget->display, sl->widget->window);
}

void Slider_SetMinSize(Slider *sl, int min_size){
    sl->min_size = min_size;
}

void Slider_SetSize(Slider *sl, int size){
    if(size < sl->min_size) sl->size = sl->min_size;
    else sl->size = size;
    sl->slid_y = DEF_MINSIZE;
    sl->slid_x = DEF_MINSIZE + sl->size;
    Slider_Paint(sl, NULL);
}

void Slider_SetStep(Slider *sl, float step){
    sl->step = step;
}
