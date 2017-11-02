#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include "../Logic/system.h"
#include "form.h"
#include "keycompose.h"
#include "textline.h"

#define DEF_RADIUS 10
#define DEF_BACKCOLOR Color_GetColor(255, 255, 255)
#define DEF_BORDER

void Paint(void *arg){
    TextLine *tl = (TextLine*)arg;
    TextLine_Paint(tl, NULL);
}

void *DrawCur(void *arg){
    TextLine *tl = (TextLine*)arg;
    Form *f = NULL;
    S_Widget *w = tl->widget->parent;
    while(w != NULL && w->type != WIDGET_TYPE_FORM){
        w = w->parent;
    }
    if(w == NULL) return NULL;
    f = (Form*)w->object;
    Event ev;
    ev.func = Paint;
    ev.arg = arg;
    while(!tl->stop){
        thread_pause(500);
        if(tl->focus && !tl->cur_visible) tl->cur_visible = 1;
        else tl->cur_visible = 0;
        Form_AddEvent(f, ev);
        //EventStack_Push(&f->evStack, ev);
    }
    return NULL;
}

TextLine *TextLine_newTextLine(S_Widget *parent, int x, int y, int width, int height, const char *def_text, char radius){
    TextLine *tl = (TextLine*)malloc(sizeof(TextLine));
    tl->widget = Widget_newWidget(x, y, width, height, parent);
    tl->widget->type = WIDGET_TYPE_TEXTLINE;
    tl->widget->object = (void*)tl;
    tl->cur_pos = tl->focus = tl->cur_visible = tl->text_count = tl->hover = 0;
    tl->def_text = def_text;
    tl->text = NULL;
    tl->radius = radius;
    tl->stop = 0;
    tl->text_pos = 0;
    tl->shift = 0;
    tl->defCursor = 0;
    tl->focCursor = 0;
    tl->EnterPress = NULL;
    tl->obj = NULL;
    if(radius) tl->cur_pos = DEF_RADIUS;

    XSelectInput(tl->widget->display, tl->widget->window, ExposureMask | KeyPressMask | ButtonPressMask | FocusChangeMask |
                 EnterWindowMask | LeaveWindowMask);

#if defined(OC2K1x)
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = 5;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&tl->th, &attr, DrawCur, (void*)tl);
#else
    pthread_create(&tl->th, NULL, DrawCur, (void*)tl);
#endif
    return tl;
}

void TextLine_Dispose(TextLine *tl){
    if(tl->text) free(tl->text);
    tl->stop = 1;
    pthread_join(tl->th, NULL);
    //if(!tl->widget->disposed) Widget_Dispose(tl->widget);
    free(tl);
}

void TextLine_Paint(TextLine *tl, PaintEventArgs *ev){
    Graphics *g = tl->widget->g;
    Display *d = g->widget->display;
    Graphics_BeginPaint(g, ev);
    Color backcolor = tl->widget->parent->backgroundColor;
    Graphics_SetColor(g, backcolor);
    Graphics_FillRectangle(g, 0, 0, tl->widget->width, tl->widget->height);
    if(tl->focus) backcolor = Color_GetColor1(130, 130, 210, d);
    else backcolor = Color_GetColor1(190, 190, 190, d);
    Graphics_SetColor(g, backcolor);
    if(tl->radius){
        Graphics_DrawRectangle(g, DEF_RADIUS, 0, tl->widget->width - (DEF_RADIUS<<1), tl->widget->height-2);
        Graphics_SetColor(g, WHITE_COLOR(d));
        Graphics_FillEllips(g, DEF_RADIUS, DEF_RADIUS, DEF_RADIUS<<1, DEF_RADIUS<<1);
        Graphics_FillEllips(g, tl->widget->width - DEF_RADIUS - 1, DEF_RADIUS, DEF_RADIUS<<1, DEF_RADIUS<<1);
        Graphics_SetColor(g, backcolor);
        Graphics_DrawEllips(g, DEF_RADIUS, DEF_RADIUS, DEF_RADIUS<<1, DEF_RADIUS<<1);
        Graphics_DrawEllips(g, tl->widget->width - DEF_RADIUS - 1, DEF_RADIUS, DEF_RADIUS<<1, DEF_RADIUS<<1);
        Graphics_SetColor(g, WHITE_COLOR(d));
        Graphics_FillRectangle(g, DEF_RADIUS, 1, tl->widget->width - (DEF_RADIUS<<1), tl->widget->height - 3);
        if(tl->text == NULL && tl->def_text != NULL){
            backcolor = Color_GetColor1(190, 190, 190, d);
            Graphics_SetColor(g, backcolor);
            Graphics_SetFontSize(g, 14);
            Graphics_DrawText(g, DEF_RADIUS, tl->widget->height - 4, tl->def_text);
        }
        else if(tl->text != NULL){
            Graphics_SetColor(g, BLACK_COLOR(d));
            Graphics_SetFontSize(g, 14);
            char *text_to_draw = &tl->text[tl->shift];
            int tt, n = strlen(text_to_draw);
            while((tt = XTextWidth(g->fontInfo, text_to_draw, n) > tl->widget->width - 18)) n--;
            text_to_draw = (char*)calloc(n+1, 1);
            strncpy(text_to_draw, &tl->text[tl->shift], n);
            Graphics_DrawText(g, DEF_RADIUS, tl->widget->height - 4, text_to_draw);
            free(text_to_draw);
        }
        if(tl->focus && tl->cur_visible){
            Graphics_SetColor(g, BLACK_COLOR(d));
            Graphics_SetLineWidth(g, 1);
            Graphics_DrawLine(g, tl->cur_pos, 4, tl->cur_pos, tl->widget->height - 5);
        }
    }
    else{
        Graphics_DrawRectangle(g, 0, 0, tl->widget->width, tl->widget->height);
        Graphics_SetColor(g, WHITE_COLOR(d));
        Graphics_FillRectangle(g, 0, 0, g->widget->width, g->widget->height);
    }
    Graphics_EndPaint(g);
}

void TextLine_MouseEnter(TextLine *tl, MouseEventArgs *ev){
    if(tl->focCursor == 0) tl->focCursor = XCreateFontCursor(tl->widget->display, XC_xterm);
    XUndefineCursor(tl->widget->display, tl->widget->window);
    XDefineCursor(tl->widget->display, tl->widget->window, tl->focCursor);
}

void TextLine_MouseLeave(TextLine *tl, MouseEventArgs *ev){
    if(tl->defCursor == 0) tl->defCursor = XCreateFontCursor(tl->widget->display, XC_left_ptr);
    XUndefineCursor(tl->widget->display, tl->widget->window);
    XDefineCursor(tl->widget->display, tl->widget->window, tl->defCursor);
}

void TextLine_MouseDown(TextLine *tl, MouseEventArgs *ev){
    if(ev->win == tl->widget->window){
        Widget_Tab(tl->widget);
        //tl->focus = 1;
        if(tl->text != NULL){
            int t_width = XTextWidth(tl->widget->g->fontInfo, tl->text, strlen(tl->text));
            if(ev->x > t_width){
                tl->text_pos = strlen(tl->text);
                tl->cur_pos = t_width + 11;
            }
            else if(ev->x > 11 && ev->x < t_width){
                int pos = ev->x - 11;
                int t_l = 0;
                int t_w = 0;
                //while((t_w = XTextWidth(tl->widget->g->fontInfo, tl->text, ++t_l)) < pos);
                while((t_w = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], ++t_l)) < pos);
                tl->text_pos = t_l + tl->shift;
                tl->cur_pos = t_w + 11;
            }
            else if(ev->x > 0){
                tl->cur_pos = 11;
                if(tl->shift > 0) tl->text_pos = tl->shift;
                else tl->text_pos = 0;
            }
        }
        //tl->cur_visible = 1;

    }
    else{
        //tl->focus = 0;
        //tl->cur_visible = 0;
    }
    TextLine_Paint(tl, NULL);
}

void TextLine_KeyPress(TextLine *tl, KeyEventArgs *ev){
    if(tl->focus){
        switch(ev->ksym){
        case XK_Insert:
        case XK_Page_Up:
        case XK_Page_Down:
        case XK_Up:
        case XK_Down:
        case XK_Print:
        case XK_Pause:
        case XK_Break:
        case XK_plusminus:
        case XK_Tab:
        case XK_KP_Enter:
        case XK_KP_Add:
        case XK_KP_Begin:
        case XK_KP_Decimal:
        case XK_KP_Divide:
        case XK_KP_Down:
        case XK_KP_Equal:
        case XK_KP_Page_Down:
        case XK_KP_Page_Up:
        case XK_KP_Insert:
        case XK_KP_Multiply:
        case XK_KP_Up:
        case XK_KP_Subtract:
        case XK_Num_Lock:
        case XK_Shift_L:
        case XK_Shift_R:
        case XK_Caps_Lock:
        case XK_Control_L:
        case XK_Control_R:
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Menu:
        case XK_Escape:
        case XK_F1:
        case XK_F2:
        case XK_F3:
        case XK_F4:
        case XK_F5:
        case XK_F6:
        case XK_F7:
        case XK_F8:
        case XK_F9:
        case XK_F10:
        case XK_F11:
        case XK_F12:
        case XK_F13:
        case XK_F14:
        case XK_Scroll_Lock:
            break;
        case XK_Return:
            if(tl->EnterPress) tl->EnterPress(tl->obj, tl->text);
            break;
        case XK_Home:
        case XK_KP_Home:
            tl->text_pos = 0;
            tl->shift = 0;
            tl->cur_pos = 10;
            TextLine_Paint(tl, NULL);
            break;
        case XK_End:
        case XK_KP_End:
            if(tl->text_count > 0){
                tl->text_pos = tl->text_count;
                tl->shift = 0;
                do{
                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->text_count - tl->shift) + 11;
                }while(tl->cur_pos >= tl->widget->width - 18 && ++tl->shift);
                TextLine_Paint(tl, NULL);
            }
            break;
        case XK_BackSpace:
            if(tl->text != NULL && tl->text_pos > 0){
                if(tl->text_count == 1){
                    free(tl->text);
                    tl->text = NULL;
                    tl->text_count = tl->text_pos = 0;
                    tl->cur_pos = 10;
                }
                else{
                    char *text = tl->text;
                    tl->text = (char*)malloc(tl->text_count);
                    strncpy(tl->text, text, tl->text_pos-1);
                    strncpy(&tl->text[tl->text_pos-1], &text[tl->text_pos], tl->text_count - tl->text_pos);
                    tl->text_pos--;
                    tl->text_count--;
                    tl->text[tl->text_count] = 0;
                    if(tl->shift > 0) tl->shift--;
                    //tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_count - tl->shift) + 11;
                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->text_pos - tl->shift)+11;
                }
            }
            TextLine_Paint(tl, NULL);
            break;
        case XK_Delete:
        case XK_KP_Delete:
            if(tl->text != NULL && tl->text_pos < tl->text_count){
                if(tl->text_count == 1){
                    free(tl->text);
                    tl->text = NULL;
                    tl->text_count = tl->text_pos = 0;
                    tl->cur_pos = 10;
                }
                else{
                    char *text = tl->text;
                    tl->text = (char*)malloc(tl->text_count);
                    strncpy(tl->text, text, tl->text_pos);
                    strncpy(&tl->text[tl->text_pos], &text[tl->text_pos+1], tl->text_count - (tl->text_pos+1));
                    tl->text_count--;
                    tl->text[tl->text_count] = 0;
                    if(tl->shift > 0) tl->shift--;
                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->text_pos - tl->shift) + 11;
                }
                TextLine_Paint(tl, NULL);
            }
            break;
        case XK_Left:
        case XK_KP_Left:
            if(tl->text != NULL && tl->text_pos > 0){
                tl->text_pos--;
                tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift);
                if(tl->cur_pos < 0){
                    tl->shift--;
                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
                }
                else tl->cur_pos += 11;
                TextLine_Paint(tl, NULL);
            }
            break;
        case XK_Right:
        case XK_KP_Right:
            if(tl->text != NULL && tl->text_pos < strlen(tl->text)){
                tl->text_pos++;
                tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
                if(tl->cur_pos - 11 > tl->widget->width - 18){
                    tl->shift++;
                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
                }
                TextLine_Paint(tl, NULL);
            }
            break;
        default:
            ev->keyChar = ComposeKeyEn_ToRu(ev->keyChar, ev->shift_mod || ev->caps_mod ? 1 : 0);
            if(tl->text == NULL){
                tl->text = (char*)malloc(2);
                tl->text[tl->text_count++] = ev->keyChar;
                tl->text_pos++;
            }
            else{
                char *text = tl->text;
                tl->text = (char*)malloc(tl->text_count + 2);
                strncpy(tl->text, text, tl->text_pos);
                tl->text[tl->text_pos++] = ev->keyChar;
                strncpy(&tl->text[tl->text_pos], &text[tl->text_pos-1], tl->text_count - (tl->text_pos - 1));
                tl->text_count++;
                free(text);
            }
            tl->text[tl->text_count] = 0;
            tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;

//            if(tl->shift > 0){
//                shift_x = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->shift);
//            }
            if(tl->cur_pos - 11 > tl->widget->width - 18){
                tl->shift ++;
                tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
            }
            TextLine_Paint(tl, NULL);
            break;
        }

//        if(ev->ksym == XK_Return){
//            if(tl->EnterPress) tl->EnterPress(tl->obj, tl->text);
//        }
//        else if(ev->ksym == XK_Home){
//            tl->text_pos = 0;
//            tl->shift = 0;
//            tl->cur_pos = 10;
//            TextLine_Paint(tl, NULL);
//        }
//        else if(ev->ksym == XK_End){
//            if(tl->text_count > 0){
//                tl->text_pos = tl->text_count;
//                tl->shift = 0;
//                do{
//                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->text_count - tl->shift) + 11;
//                }while(tl->cur_pos >= tl->widget->width - 18 && ++tl->shift);
//                TextLine_Paint(tl, NULL);
//            }
//        }
//        else if(ev->ksym == XK_BackSpace){
//            if(tl->text != NULL && tl->text_pos > 0){
//                if(tl->text_count == 1){
//                    free(tl->text);
//                    tl->text = NULL;
//                    tl->text_count = tl->text_pos = 0;
//                    tl->cur_pos = 10;
//                }
//                else{
//                    char *text = tl->text;
//                    tl->text = (char*)malloc(tl->text_count);
//                    strncpy(tl->text, text, tl->text_pos-1);
//                    strncpy(&tl->text[tl->text_pos-1], &text[tl->text_pos], tl->text_count - tl->text_pos);
//                    tl->text_pos--;
//                    tl->text_count--;
//                    tl->text[tl->text_count] = 0;
//                    if(tl->shift > 0) tl->shift--;
//                    //tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_count - tl->shift) + 11;
//                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->text_pos - tl->shift)+11;
//                }
//            }
//            TextLine_Paint(tl, NULL);
//        }
//        else if(ev->ksym == XK_Left){
//            if(tl->text != NULL && tl->text_pos > 0){
//                tl->text_pos--;
//                tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift);
//                if(tl->cur_pos < 0){
//                    tl->shift--;
//                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
//                }
//                else tl->cur_pos += 11;
//                TextLine_Paint(tl, NULL);
//            }
//        }
//        else if(ev->ksym == XK_Right){
//            if(tl->text != NULL && tl->text_pos < strlen(tl->text)){
//                tl->text_pos++;
//                tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
//                if(tl->cur_pos - 11 > tl->widget->width - 18){
//                    tl->shift++;
//                    tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
//                }
//                TextLine_Paint(tl, NULL);
//            }
//        }
//        else if(ev->ksym == XK_Up || ev->ksym == XK_Down) return;
//        else if((unsigned char)ev->keyChar >= 30 && (unsigned char)ev->keyChar <= 127) {
//            char bs = ev->shift_mod || ev->caps_mod ? 1 : 0;
//            ev->keyChar = ComposeKeyEn_ToRu(ev->keyChar, bs);
//            if(tl->text == NULL){
//                tl->text = (char*)malloc(2);
//                tl->text[tl->text_count++] = ev->keyChar;
//                tl->text_pos++;
//            }
//            else{
//                char *text = tl->text;
//                tl->text = (char*)malloc(tl->text_count + 2);
//                strncpy(tl->text, text, tl->text_pos);
//                tl->text[tl->text_pos++] = ev->keyChar;
//                strncpy(&tl->text[tl->text_pos], &text[tl->text_pos-1], tl->text_count - (tl->text_pos - 1));
//                tl->text_count++;
//                free(text);
//            }
//            tl->text[tl->text_count] = 0;
//            tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;

////            if(tl->shift > 0){
////                shift_x = XTextWidth(tl->widget->g->fontInfo, tl->text, tl->shift);
////            }
//            if(tl->cur_pos - 11 > tl->widget->width - 18){
//                tl->shift ++;
//                tl->cur_pos = XTextWidth(tl->widget->g->fontInfo, &tl->text[tl->shift], tl->text_pos - tl->shift) + 11;
//            }
//            TextLine_Paint(tl, NULL);
//        }
    }
}

void TextLine_FocusIn(TextLine *tl, EventArgs *ev){
    tl->cur_visible = 1;
    TextLine_Paint(tl, NULL);
}

void TextLine_FocusOut(TextLine *tl, EventArgs *ev){
    tl->cur_visible = 0;
    TextLine_Paint(tl, NULL);
}

void TextLine_Tab(TextLine *tl){
    tl->focus = 1;
    TextLine_Paint(tl, NULL);
}

void TextLine_Untab(TextLine *tl){
    tl->focus = 0;
    TextLine_Paint(tl, NULL);
}
