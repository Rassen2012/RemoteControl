#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scrollbar.h"

#define DEF_SIZE 15
#define DEF_SLIDSIZE 10
#define DEF_SLIDX 3
#define DEF_MINX DEF_SIZE
#define DEF_MINY DEF_SIZE
#define DEF_MINSIZE 30
#define DEF_SLIDSTEP 1

//void Scroll_CalcStep(Scroll *s){
//    s->step = 1.0f;
//    if(s->type == VERTICAL_SCROLL){
//        if(s->sw->height > s->widget->height){
//            float m_height = s->widget->height - (DEF_SIZE<<1);
//            float ssize = m_height - (s->sw->height - s->widget->height);
//            if(ssize < DEF_MINSIZE){
//                s->step = (DEF_MINSIZE - ssize)/(m_height - DEF_MINSIZE);
//                ssize = DEF_MINSIZE;
//            }
//            s->slid_size = ssize;
//            Scroll_Paint(s, NULL);
//        }
//    }

//}

//void Scroll_MoveSw(Scroll *s, float val){
//    if(s->mov != NULL){
//        val = val * s->step + s->ost;
//        double tval = (double)val;
//        s->ost = modf(tval, &tval);
//        s->mov((void*)s, (int)tval);
//    }
//}

void Scroll_VMove(void *obj, float val){
    Scroll *s = (Scroll*)(((Slider*)obj)->widget->parent->object);
    if(s->sw == NULL) return;

    int hheight = s->widget->height;
    if(s->type & HORIZONTAL_SCROLL && s->slHor->widget->visible) hheight -= 15;
    float all_per = hheight - s->sw->height;//s->sw->height - s->widget->height;
    float cur_pos = (val * all_per)/100.0f;
    double dval = cur_pos + s->v_ost;
    s->v_ost = modf(dval, &dval);
    if(dval > 0) dval = 0;
    else if(dval < hheight - s->sw->height) dval = hheight - s->sw->height;
    Widget_Move(s->sw, s->sw->x, dval);

    /*double dval = val + s->v_ost;
    s->v_ost = modf(dval, &dval);
    dval = s->sw->y - dval;
    if(dval > 0) dval = 0;
    else if(dval < s->widget->height - s->sw->height) dval = s->widget->height - s->sw->height;
    Widget_Move(s->sw, s->sw->x, dval);*/
}

void Scroll_HMove(void *obj, float val){
    Scroll *s = (Scroll*)(((Slider*)obj)->widget->parent->object);
    if(s->sw == NULL) return;

    float all_per = s->widget->width - s->sw->width;
    float cur_pos = (val * all_per)/100.0f;
    double dval = cur_pos + s->h_ost;
    s->h_ost = modf(dval, &dval);
    if(dval > 0) dval = 0;
    else if(dval < s->widget->width - s->sw->width) dval = s->widget->width - s->sw->width;
    Widget_Move(s->sw, dval, s->sw->y);
}

Scroll *Scroll_newScroll(S_Widget *parent, int x, int y, int width, int height, SCROLL_TYPES type){
    Scroll *s = (Scroll*)malloc(sizeof(Scroll));
    s->type = type;
    s->widget = Widget_newWidget(x, y, width, height, parent);
    s->widget->is_tabed = 0;
    s->widget->type = WIDGET_TYPE_SCROLLBAR;
    s->widget->object = (void*)s;
    s->h_ost = s->v_ost = 0;
    s->slHor = s->slVer = NULL;
    int hheight = s->widget->height;
    if(type & VERTICAL_SCROLL){
        //if(type & HORIZONTAL_SCROLL) hheight -= 15;
        s->slVer = Slider_newSlider(s->widget, SLIDER_TYPE_VSCROLL, s->widget->width - 15, 0, 15, hheight, Scroll_VMove);
        Widget_SetVisibility(s->slVer->widget, 0);
    }
    if(type & HORIZONTAL_SCROLL){
        s->slHor = Slider_newSlider(s->widget, SLIDER_TYPE_HSCROLL, 0, s->widget->height - 15, s->widget->width, 15, Scroll_HMove);
        Widget_SetVisibility(s->slHor->widget, 0);
    }
    s->sw = NULL;
    return s;
}


void Scroll_Dispose(Scroll *s){
    if(!s->widget->disposed) Widget_Dispose(s->widget);
    free(s);
}

void Scroll_Paint(Scroll *s, PaintEventArgs *ev){
    Graphics *g = s->widget->g;
    Graphics_BeginPaint(g, ev);
    Color backcolor = Color_GetColor1(0, 0, 0, g->widget->display);
    Graphics_SetColor(g, backcolor);
    Graphics_EndPaint(g);
}

void Scroll_ResizeEvent(Scroll *s, ResizeEventArgs *ev){
    int width = s->widget->width + (ev->width - ev->old_width);
    if(width < 0) width = ev->width;
    int height = s->widget->height + (ev->height - ev->old_height);
    if(height < 0) height = ev->height;
    //Widget_Resize(s->widget, width, height);
    Widget_Resize(s->widget, ev->width, ev->height);
    //Scroll_SetWidget(s, s->sw);
}

void Scroll_SetWidget(Scroll *s, S_Widget *sw){
    s->sw = sw;

    Slider *sl = NULL;
    //if(s->type & HORIZONTAL_SCROLL) Widget_Show(s->slHor->widget);
    s->v_ost = s->h_ost = 0;
    if(s->widget->height < sw->height){
        int sl_size = sw->height - s->widget->height;
        if(s->type & VERTICAL_SCROLL){
            sl = s->slVer;
            sl_size = (s->widget->height - (DEF_SIZE<<1)) - sl_size;
            if(sl_size < DEF_MINSIZE){
//                int val1 = DEF_MINSIZE - sl_size;
//                int val2 = sw->height - s->widget->height;
//                Slider_SetStep(sl, 1 + (float)(val1)/(float)(val2));
//                printf("set step %f\n", sl->step);
                sl_size = DEF_MINSIZE;
            }
            //else Slider_SetStep(sl, 1);
            Slider_SetSize(sl, sl_size);
            if(!sl->widget->visible) Widget_Show(sl->widget);
        }
    }
    else{
        if(s->type & VERTICAL_SCROLL){
            sl = s->slVer;
            Slider_SetSize(sl, s->widget->height - (DEF_SIZE<<1));
            if(sl->widget->visible) Widget_Hide(sl->widget);
        }
    }
    if(s->widget->width < sw->width){
        int sl_size = sw->width - s->widget->width;
        if(s->type & HORIZONTAL_SCROLL){
            sl = s->slHor;
            sl_size = (s->widget->width - (DEF_SIZE<<1)) - sl_size;
            if(sl_size < DEF_MINSIZE) sl_size = DEF_MINSIZE;
            Slider_SetSize(sl, sl_size);
            if(!sl->widget->visible){
                Widget_SetVisibility(sl->widget, 1);
                Widget_Show(sl->widget);
                if(s->type & VERTICAL_SCROLL){
                    Widget_Resize(s->slVer->widget, s->slVer->widget->width, s->slVer->widget->height - DEF_SIZE);
                    Scroll_SetWidget(s, sw);
                }
            }
        }
    }
    else{
        if(s->type & HORIZONTAL_SCROLL){
            sl = s->slHor;
            Slider_SetSize(sl, s->widget->width - (DEF_SIZE<<1));
            if(sl->widget->visible){
                Slider_MovePer(sl, -100);
                Widget_SetVisibility(sl->widget, 1);
                Widget_Hide(sl->widget);
                if(s->type & VERTICAL_SCROLL){
                    Widget_Resize(s->slVer->widget, s->slVer->widget->width, s->slVer->widget->height + DEF_SIZE);
                    Scroll_SetWidget(s, sw);
                }
            }
        }
    }
}

