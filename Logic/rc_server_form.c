#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef linux
#include <malloc.h>
#endif
#include <X11/keysym.h>
#include <limits.h>
#include <X11/cursorfont.h>

#include "minilzo/minilzo.h"
#include "public.h"
#if defined (OC2K1x)
#include "general.h"
#else
#include "system.h"
#endif
#include "rc_server_form.h"
//#include "Cursor.xbm"
//#include "Cursor_mask.xbm"


#define BIG_PAL_SIZE 65536//262144//16777216
#define TITLE_HEIGHT 20
#define RESIZE_TIME 1000

char ctrlPressed = 0;

void *MainThread(void *arg);
void *ImageThread(void *arg);

void MouseMove(Form *f, MouseEventArgs *ev);
void MousePress(Form *f, MouseEventArgs *ev);
void MouseRelease(Form *f, MouseEventArgs *ev);
void KeyDown(Form *f, KeyEventArgs *ev);
void KeyUp(Form *f, KeyEventArgs *ev);
void ResizeEvent(Form *f, ResizeEventArgs *ev);
void RCServerForm_Paint(Form *f, PaintEventArgs *ev);
//void RCServerForm_PackImage(RCServerForm *sf, char *src, ImageInfo *imInfo, char **old);
void *RCServerForm_PackImage(void *arg);
void RCServerForm_Close(Form *f);

typedef struct ImagePackInfo{
    int width, height;
    int depth;
    int size;
    char *buf;
    char rle;
    int cur_height;
}ImagePackInfo;

void RCServerForm_Close(Form *f){
    RCServerForm *sf = (RCServerForm*)f->obj;
    RCServerForm_Dispose(sf);
}

void RCServerForm_Raise(RCServerForm *sf){
//    Event ev;
//    ev.type = EV_RAISE_AND_FOCUS;//EV_RAISE_WINDOW;
//    ev.func = NULL;
//    pthread_mutex_lock(&sf->f->evs_mutex);
//    EventStack_Push(&sf->f->evStack, ev);
//    pthread_mutex_unlock(&sf->f->evs_mutex);
}

void RCServerForm_Enter(Form *f){
#if defined(OC2K1x)
    return;
#endif
//    Pixmap curPix, curMask;
//    int width = 24, height = 34;
//    //int width = 50, height = 50;
//    curPix = XCreateBitmapFromData(f->widget->display, f->widget->window, Cursor_bits, width, height);
//    curMask = XCreateBitmapFromData(f->widget->display, f->widget->window, Cursor_mask_bits, width, height);
//    XColor fc, bc;
//    XAllocNamedColor(f->widget->display, XDefaultColormap(f->widget->display, XDefaultScreen(f->widget->display)), "black", &fc, &fc);
//    XAllocNamedColor(f->widget->display, XDefaultColormap(f->widget->display, XDefaultScreen(f->widget->display)), "white", &bc, &bc);
//    f->cur = XCreatePixmapCursor(f->widget->display, curPix, curMask, &fc, &bc, 5, 5);
//    XDefineCursor(f->widget->display, f->widget->window, f->cur);
}

void RCServerForm_Leave(Form *f){
#if defined(OC2K1x)
    return;
#endif
//    if(f->cur != 0){
//        XUndefineCursor(f->widget->display, f->widget->window);
//        XFreeCursor(f->widget->display, f->cur);
//        f->cur = 0;
//    }
}

void RCServerForm_RleDecompress3(char **old, char *src, int src_size, char *dst, int *dst_size, int n){
    int i, s = 0, v = 0;
    unsigned char sym_count = 0;
    unsigned int cur = 0;
    unsigned char val = 0;
    char *t = *old;
    //char *t = old;
    size_t step = n;
    if(step == 3){
        while(s < src_size){
            val = (unsigned char)src[s++];
            sym_count = (unsigned char)src[s++];
            if(val){
                memcpy(&cur, &src[s], step);
                s+=step;
                for(i = 0; i < sym_count; i++){
                    memcpy(&dst[v], &cur, step);
                    v+=step;
                }
            }
            else{
                memcpy(&dst[v], &t[v], sym_count*step);
                v+=sym_count*step;
            }
        }
    }
    else if(step == 4){// || step == 3){
        while(s < src_size){
            val = (unsigned char)src[s++];
            if(val){
                sym_count = (unsigned char)src[s++];
                memcpy(&cur, &src[s], step);
                //memset(&dst[v], cur, sym_count*step);
                for(i = 0; i < sym_count; i++){
                    memcpy(&dst[v], &cur, step);
                    v+=step;
                }
                s+=step;
                //v+=sym_count*step;
            }
            else{
                sym_count = (unsigned char)src[s++];
                memcpy(&dst[v], &t[v], sym_count*step);
                v+=(sym_count*step);
            }
        }
    }
    else if(step == 2){
        while(s < src_size){
            val = (unsigned char)src[s++];
            sym_count = (unsigned char)src[s++];
            if(val){
                //memcpy(&cur, &src[s], step);
                for(i = 0; i < sym_count; i++){
                    //memcpy(&dst[v], &cur, step);
                    memcpy(&dst[v], &src[s], step);
                    v+=step;
                }
                s+=step;
            }
            else{
                memcpy(&dst[v], &t[v], sym_count*step);
                v+=sym_count*step;
            }
        }
    }
    else{
        while(s < src_size){
            val = (unsigned char)src[s++];
            sym_count = (unsigned char)src[s++];
            if(val){
                cur = (unsigned char)src[s++];
                //for(i = 0; i < sym_count; i++){
                //    dst[v++] = cur;
                //}
                memset(&dst[v], cur, sym_count);
                v+=sym_count;
            }
            else{
                memcpy(&dst[v], &t[v], (size_t)sym_count);
                v+=sym_count;
            }
        }
    }


    free(t);
    t = (char*)malloc(sizeof(char)*v);
    memcpy(t, dst, v);
    *old = t;
    //old = t;
    *dst_size = v;
}

char *RCServerForm_RedepthImage16x24(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*size*2);
    int i;//, j;
    unsigned int c;
    //size_t step = sizeof(unsigned int);
    unsigned short *it_i = (unsigned short*)src;
    unsigned int *it_d = (unsigned int*)dst;
    for(i = 0; i < size; i+=2){
        c = *it_i++;

        c = ((c>>8)&0xf8)<<16 | ((c>>3)&0xfc)<<8 | (c&0x1f)<<3;

        //c = ((c<<3) & 0xf8) | ((c>>2)&0x7) | ((c<<5) & 0xfc00) | ((c>>1) & 0x300) | ((c<<8) & 0xf80000) | ((c<<3) & 0x70000);
        //c = Color_24From16(c).l;
        *it_d++ = c;
    }


    //    for(i = 0, j = 0; i < size; i+=2, j+=4){
    //        memcpy(&c, &src[i], step);
    //        c = ((c<<3) & 0xf8) | ((c>>2)&0x7) | ((c<<5) & 0xfc00) | ((c>>1) & 0x300) | ((c<<8) & 0xf80000) | ((c<<3) & 0x70000);
    //        memcpy(&dst[j], &c, step);
    //    }
    return dst;
}

char *RCServerForm_RedepthImage8x24(char *src, int size, RCServerForm* sf){
    char *dst = (char*)malloc(sizeof(char)*size*4);
    int i, j;
    unsigned int c;
    unsigned char r, g, b;
    size_t step = 4;
    for(i = 0, j = 0; i < size; i++, j+=4){
        r = sf->pal[(unsigned char)src[i]][0]>>8;
        g = sf->pal[(unsigned char)src[i]][1]>>8;
        b = sf->pal[(unsigned char)src[i]][2]>>8;
        c = (r<<16)|(g<<8)|b;
        memcpy(&dst[j], &c, step);
    }
    return dst;
}

char *RCServerForm_RedepthImage8x16_1(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*(size/2));
    int i;
    unsigned int c;
    unsigned char r, g, b;
    unsigned short *dstx = (unsigned short*)dst;
    //size_t step = 2;
    for(i = 0; i < size; i+=4){
        r = src[i+2];//(unsigned char)(ci->cm[(unsigned char)src[i]][0]>>8);
        g = src[i+1];//(unsigned char)(ci->cm[(unsigned char)src[i]][1]>>8);
        b = src[i];//(unsigned char)(ci->cm[(unsigned char)src[i]][2]>>8);
        c = (r<<16)|(g<<8)|b;
        c = Color_16From24(c);
        *dstx++ = (unsigned short)c;
        //memcpy(&dst[j], &c, step);
    }
    return dst;
}

char *RCServerForm_RedepthImage8x16(char *src, int size, RCServerForm* sf){
    size_t step = 2;
    char *dst = (char*)malloc(sizeof(char)*size*step);
    int i, j;
    unsigned int c;
    unsigned char r, g, b;
    for(i = 0, j = 0; i < size; i++, j += step){
        r = sf->pal[(unsigned char)src[i]][0]>>8;
        g = sf->pal[(unsigned char)src[i]][1]>>8;
        b = sf->pal[(unsigned char)src[i]][2]>>8;
        c = (r<<16) | (g<<8) | b;
        //c = (r<<11)|((g&0x3f)<<5)|(b&0x1f);
        c = Color_16From24(c);
        memcpy(&dst[j], &c, step);
    }
    return dst;
}

char *RCServerForm_RedepthImage24x16(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*(size/2));
    int i, j;
    unsigned int c;
    size_t step = sizeof(unsigned int);
    for(i = 0, j = 0; i < size; i+=4, j+=2){
        memcpy(&c, &src[i], step);
        c = Color_16From24(c);
        memcpy(&dst[j], &c, step);
    }
    return dst;
}

int RCServerForm_GetFixedColor(unsigned int c, RCServerForm *sf){
    int i, index, val1, val2, val3;
    index = 0;
    val3 = -1;
    val2 = 0;
    val1 = c;
    for(i = 0; i < 256; i++){
        val2 = ((unsigned char)(sf->serv_pal[i][0]>>8) << 16) | ((unsigned char)(sf->serv_pal[i][1]>>8) << 8) | (unsigned char)(sf->serv_pal[i][2]>>8);
        if(abs(val1 - val2) < val3 || val3 < 0){
            index = i;
            val3 = abs(val1 - val2);
            if(val3 == 0) break;
        }
    }
    return index;
}

int RCServerForm_GetFixedColor16(unsigned int c, RCServerForm *sf){
    int i, index, val1, val2, val3;
    index = 0;
    val3 = -1;
    val2 = 0;
    val1 = c;
    for(i = 0; i < 256; i++){
        val2 = ((unsigned char)(sf->serv_pal[i][0]>>8) <<11) | (((unsigned char)(sf->serv_pal[i][1]>>8)&0x3f) << 5) | ((unsigned char)(sf->serv_pal[i][2]>>8)&0x1f);
        val2 &= 0x0000ffff;
        if(abs(val1 - val2) < val3 || val3 < 0){
            index = i;
            val3 = abs(val1 - val2);
            if(val3 == 0) break;
        }
    }
    return index;
}

char *RCServerForm_RedepthImage24x8(char *src, int size, RCServerForm *sf){
    char *dst = (char*)malloc(sizeof(char)*size/4);
    int i, j, val;
    unsigned int *it = (unsigned int*)src;
    for(i = 0, j = 0; i < size; i+=4, it++){
        val = (((*it>>19)&0x1f)<<11) | (((*it>>10)&0x3f)<<5) | ((*it>>3)&0x1f);
        dst[j++] = sf->big_pal[val];
    }
    //    for(i = 0, j = 0; i < size; i+=4, it++){
    //        dst[j++] = sf->big_pal[(((*it>>16)&0xcf)<<12) | (((*it>>8)&0xcf)<<6) | (*it&0xcf)];
    //    }
    //    for(i = 0, j = 0; i < size; i+=4){
    //        dst[j++] = sf->big_pal[*it++];
    //    }
    /*for(i = 0, j = 0; i < size; i+=3, j++){
        dst[j] = GetApPalete(src[i+2], src[i+1], src[i], sf);
    }*/
    return dst;
}

char *RCServerForm_RedepthImage8x24_1(char *src, int size){
    //return;
    char *dst = (char*)malloc(sizeof(char)*size);// + size/3));//*4);
    /*int i, j;
    unsigned int c;
    unsigned char r, g, b;
    unsigned int *it = (unsigned int*)dst;
    unsigned int *it_o = (unsigned int*)src;
    //size_t step = 4;
    for(i = 0, j = 0; i < size; i+=4, j+=4){
        r = src[i+2];//(unsigned char)(ci->cm[(unsigned char)src[i]][0]>>8);
        g = src[i+1];//(unsigned char)(ci->cm[(unsigned char)src[i]][1]>>8);
        b = src[i];//(unsigned char)(ci->cm[(unsigned char)src[i]][2]>>8);
        c = (r<<16)|(g<<8)|b;
        //memcpy(&dst[j], &c, step);
        *it++ = c;
    }*/
    memcpy(dst, src, size);
    return dst;
}

char *RCServerForm_RedepthImage16x8(char *src, int size, RCServerForm *sf){
    char *dst = (char*)malloc(size/2);
    int i, j;
    //unsigned int c;
    //size_t step = 1;
    unsigned short *dsrc = (unsigned short*)src;
    for(i = 0, j = 0; i < size; i+=2){
        //memcpy(&c, &src[i], 2);
        //c = RCServerForm_GetFixedColor16(c, sf);
        //memcpy(&dst[j], &c, step);
        dst[j++] = sf->big_pal[*dsrc++];
    }
    return dst;
}

void RCServerForm_DecompressImage(char *src, unsigned long src_size, char *dst, unsigned long *dst_size){
    int ret = lzo1x_decompress((unsigned char*)src,  (lzo_uint32)src_size, (unsigned char*)dst, (lzo_uintp)dst_size, 0);
    if(ret < 0) *dst_size = 0;
}

void RCServerForm_SendWindowSize(RCServerForm* sf, int width, int height){
    if(time_get() - sf->sw_time < 500) return;
    sf->sw_time = time_get();
    char buf[SHARK_BUFFSIZE] = {0};
    VisualInfo vi;
    int com = C_SET_VISUAL_INFO;
    vi.screen_width = width;
    if(sf->titleVisible) vi.screen_height = height-TITLE_HEIGHT;
    else vi.screen_height = height;
    vi.depth = Color_GetDepth();
    memcpy(buf, &com, sizeof(int));
    memcpy(&buf[sizeof(int)], &vi, sizeof(VisualInfo));
    pthread_mutex_lock(&sf->mutex);
    shark_send(sf->sh, buf, sizeof(int)+sizeof(VisualInfo));
    pthread_mutex_unlock(&sf->mutex);
}

void RCServerForm_SendGetFullImage(RCServerForm *sf){
    char buf[SHARK_BUFFSIZE] = {0};
    int com = C_GET_FULL_IMAGE;
    memcpy(buf, &com, sizeof(int));
    pthread_mutex_lock(&sf->mutex);
    shark_send(sf->sh, buf, sizeof(int));
    pthread_mutex_unlock(&sf->mutex);
}

void RCServerForm_DrawCursor(Graphics *g, PaintEventArgs *ev, int x, int y){
    //Graphics_BeginPaint(g, ev);
    XPoint p1, p2, p3, p4, p5, p6, p7;
    p1.x = x; p1.y = y;
    p2.x = x; p2.y = y+18;
    p3.x = x+4; p3.y = y+14;
    p4.x = x+7; p4.y = y+20;
    p5.x = x+10; p5.y = y+17;
    p6.x = x+8; p6.y = y+14;
    p7.x = x+11; p7.y = y+11;
    XPoint points[7] = {p1, p2, p3, p4, p5, p6, p7};
    XSetForeground(g->widget->display, g->gc, BlackPixel(g->widget->display, DefaultScreen(g->widget->display)));
    XFillPolygon(g->widget->display, g->context, g->gc, points, 7, Complex, 0);
    //Graphics_SetColor(g, Color_GetColor(255, 255, 255));
    XSetForeground(g->widget->display, g->gc, WhitePixel(g->widget->display, DefaultScreen(g->widget->display)));
    XDrawLines(g->widget->display, g->context, g->gc, points, 7, 0);
    //XSetForeground(g->widget->display, g->gc, BlackPixel(g->widget->display, DefaultScreen(g->widget->display)));
    //Graphics_SetColor(g, Color_GetColor(0, 0, 0));
    //Graphics_EndPaint(g);
}

void RCServerForm_Paint(Form *f, PaintEventArgs *ev){
    Graphics *g = f->widget->g;
    //if(f->im == NULL) return;
    int yp, xp;
    int x_t, y_t;
    RCServerForm *sf = (RCServerForm*)f->obj;
    Graphics_BeginPaint(g, NULL);
    //XPutImage(g->widget->display, g->widget->window, g->gc, f->im, 0, 0, 0, 0, f->im->width, f->im->height);
    XSetForeground(g->widget->display, g->gc, BlackPixel(g->widget->display, DefaultScreen(g->widget->display)));
    Graphics_FillRectangle(g, 0, 0, f->widget->width, f->widget->height);

    if(f->im != NULL && f->im->height < (f->widget->height-TITLE_HEIGHT)){
        if(sf->titleVisible) yp = (f->widget->height-TITLE_HEIGHT-f->im->height)>>1;
        else yp = (f->widget->height - f->im->height)>>1;
    }
    else{
        //if(sf->titleVisible) yp = TITLE_HEIGHT;
        //else yp = 0;
        yp = 0;
    }
    if(sf->titleVisible){
        XSetForeground(g->widget->display, g->gc, Color_GetColor1(0, 255, 0, g->widget->display).l);
        Graphics_FillRectangle(g, 0, 0, f->widget->width, TITLE_HEIGHT);
        yp += TITLE_HEIGHT;
        Graphics_SetColor(g, Color_GetColor1(0, 0, 0, g->widget->display));
        x_t = XTextWidth(g->fontInfo, sf->title, strlen(sf->title));
        x_t = (f->widget->width>>1)-(x_t>>1);
        y_t = TITLE_HEIGHT-5;
        Graphics_DrawText(g, x_t, y_t, sf->title);

        int stx = f->widget->width-20;
        Graphics_SetLineWidth(g, 4);
        Graphics_SetColor(g, Color_GetColor1(255, 255, 255, g->widget->display));
        Graphics_DrawLine(g, stx, 6, stx+10, 16);
        Graphics_DrawLine(g, stx+10, 6, stx, 16);
        Graphics_SetLineWidth(g, 2);
        if(sf->crHover) Graphics_SetColor(g, Color_GetColor1(120, 120, 120, g->widget->display));
        else Graphics_SetColor(g, Color_GetColor1(210, 210, 210, g->widget->display));
        Graphics_DrawLine(g, stx-1, 5, stx+10, 16);
        Graphics_DrawLine(g, stx+11, 5, stx, 16);
    }

    sf->image_y = yp;
    xp = 0;
    if(f->im != NULL && f->im->width < f->widget->width){
        xp = ((f->widget->width - f->im->width)>>1);
    }
    sf->image_x = xp;
    if(f->im != NULL) XPutImage(g->widget->display, g->context, g->gc, f->im, 0, 0, xp, yp, f->im->width, f->im->height);
    //    XCopyPlane(f->widget->display, sf->curPix, f->widget->window, f->widget->g->gc, sf->curp_x, sf->curp_y, sf->curp_width, sf->curp_height,
    //               sf->client_mouse_x, sf->client_mouse_y, 1);
    //Graphics_EndPaint(g);
    Window w1, w2;
    int xr, yr, x, y;
    unsigned long xt;
    unsigned int mask;
    xt = time_get() - sf->move_cur_time;
    XQueryPointer(f->widget->display, f->widget->window, &w1, &w2, &xr, &yr, &x, &y, &mask);
    //sf->client_mouse_y = sf->client_mouse_y - sf->image_y;
    //y = y - sf->image_y;
    //x+=sf->image_x;
    //y+=sf->image_y;
    if((xt > 1000) && (abs(sf->client_mouse_x - x) > 5 || abs(sf->client_mouse_y - y) > 5)){
        //printf("Draw cursor! xt - %u xt>1000=%d, current_time = %u, move_time = %u\n", xt,xt>tmp_val, tmp_cur_time, tmp_move_time);
        RCServerForm_DrawCursor(g, ev, sf->client_mouse_x, sf->client_mouse_y);
    }
    if(sf->disconnected){
        Graphics_SetColor(g, Color_GetColor1(200, 200, 200, g->widget->display));
        Graphics_FillRectangle(g, (g->widget->width>>1)-80, (g->widget->height>>1)-20, 160, 40);
        char *tcon = "Соединение...";
        int tx, ty;
        tx = XTextWidth(g->fontInfo, tcon, strlen(tcon));
        tx = (g->widget->width>>1)-(tx>>1);
        ty = (g->widget->height>>1)+7;
        Graphics_SetColor(g, Color_GetColor1(0, 0, 0, g->widget->display));
        Graphics_DrawText(g, tx, ty, tcon);
    }

#if defined(OC2K1x)
    Form_DrawBorder(g, f->widget->width, f->widget->height);
#endif
    Graphics_EndPaint(g);
    XSetWindowBackgroundPixmap(f->widget->display, f->widget->window, g->context);
}

void MouseMove(Form *f, MouseEventArgs *ev){
    MouseEvent mev;
    RCServerForm *sf = (RCServerForm*)f->obj;
    int type = EVENT_TYPE_MOUSE;
    char buf[SHARK_BUFFSIZE] = {0};
    //sf->move_cur_time = time_get();
    if(sf->titleVisible && sf->titlePressed){
        if(ev->button == MOUSE_BUTTON_LEFT){
            f->widget->x += ev->global_x-ev->xg_prev;
            f->widget->y += ev->global_y-ev->yg_prev;
            XMoveWindow(f->widget->display, f->widget->window, f->widget->x, f->widget->y);
        }
        return;
    }
    else if(sf->titleVisible){
        if(ev->x >= sf->f->widget->width-20 && ev->x <= sf->f->widget->width-9 && ev->y>0 && ev->y < TITLE_HEIGHT){
            sf->crHover = 1;
        }
        else sf->crHover = 0;
    }
    ev->x -= sf->image_x;
    if(ev->x <= 0) return;
    else if(f->im != NULL && ev->x > f->im->width) return;
    mev.x = ((float)ev->x*100.0)/((float)f->widget->width - (sf->image_x<<1));
    //mev.x = ((float)ev->x*100.0)/(float)f->widget->width;
    ev->y -= sf->image_y;
    if(ev->y <= 0) return;
    else if(f->im != NULL && ev->y > f->im->height) return;
    if(sf->titleVisible) mev.y = ((float)ev->y*100.0)/((float)f->widget->height - (sf->image_y+(sf->image_y-TITLE_HEIGHT)));//(float)f->widget->height;
    else mev.y = ((float)ev->y*100.0)/((float)f->widget->height - (sf->image_y<<1));
    mev.button = ev->button;
    mev.event = MOUSE_MOVE;
    memcpy(buf, &type, sizeof(int));
    memcpy(&buf[sizeof(int)], &mev, sizeof(MouseEvent));
    pthread_mutex_lock(&sf->mutex);
    if(sf->sh != NULL) shark_send(sf->sh, buf, sizeof(int)+sizeof(MouseEvent));
    pthread_mutex_unlock(&sf->mutex);
    if(ev->x > 0 && ev->x < sf->f->widget->width && ev->y > 0 && ev->y < sf->f->widget->height) sf->move_cur_time = time_get();
    //else sf->move_cur_time = 0;
}

void MousePress(Form *f, MouseEventArgs *ev){
    MouseEvent mev;
    RCServerForm *sf = (RCServerForm*)f->obj;
    int type = EVENT_TYPE_MOUSE;
    char buf[SHARK_BUFFSIZE] = {0};
    if(sf->crHover){
        Event event;
        event.func = NULL;
        event.type = EV_CLOSE_WINDOW;
        pthread_mutex_lock(&sf->f->evs_mutex);
        EventStack_Push(&sf->f->evStack, event);
        pthread_mutex_unlock(&sf->f->evs_mutex);
        sf->crHover = 0;
        return;
    }
    if(f->currentCursor != FORM_DEFAULT_CURSOR) return;
    if(sf->titleVisible && ev->y <= TITLE_HEIGHT){
        sf->titlePressed = 1;
        return;
    }
    //mev.x = ((float)ev->x*100.0)/(float)f->widget->width;
    ev->x -= sf->image_x;
    if(ev->x <= 0) return;
    else if(f->im != NULL && ev->x > f->im->width) return;
    mev.x = ((float)ev->x*100.0)/((float)f->widget->width - (sf->image_x<<1));
    ev->y = ev->y - sf->image_y;
    if(ev->y < 0) return;
    else if(f->im != NULL && ev->y > f->im->height) return;
    if(sf->titleVisible) mev.y = ((float)ev->y*100.0)/((float)f->widget->height - (sf->image_y+(sf->image_y-TITLE_HEIGHT)));
    else mev.y = ((float)ev->y*100.0)/((float)f->widget->height - (sf->image_y<<1));
    //mev.y = ((float)ev->y*100.0)/(float)f->widget->height;
    mev.button = ev->button;
    mev.event = MOUSE_PRESS;
    memcpy(buf, &type, sizeof(int));
    memcpy(&buf[sizeof(int)], &mev, sizeof(MouseEvent));
    pthread_mutex_lock(&sf->mutex);
    if(sf->sh != NULL) shark_send(sf->sh, buf, sizeof(int)+sizeof(MouseEvent));
    pthread_mutex_unlock(&sf->mutex);
}

void MouseRelease(Form *f, MouseEventArgs *ev){
    MouseEvent mev;
    RCServerForm *sf = (RCServerForm*)f->obj;
    int type = EVENT_TYPE_MOUSE;
    char buf[SHARK_BUFFSIZE] = {0};
    sf->titlePressed = 0;
    if(f->currentCursor != FORM_DEFAULT_CURSOR) return;
    if(sf->titleVisible && ev->y <= TITLE_HEIGHT) return;
    //mev.x = ((float)ev->x*100.0)/(float)f->widget->width;
    ev->x -= sf->image_x;
    if(ev->x <= 0) return;
    else if(f->im != NULL && ev->x > f->im->width) return;
    mev.x = ((float)ev->x*100.0)/((float)f->widget->width - (sf->image_x<<1));
    ev->y = ev->y - sf->image_y;
    if(ev->y < 0) return;
    else if(f->im != NULL && ev->y > f->im->height) return;
    if(sf->titleVisible) mev.y = ((float)ev->y*100.0)/((float)f->widget->height - (sf->image_y+(sf->image_y-TITLE_HEIGHT)));
    else mev.y = ((float)ev->y*100.0)/((float)f->widget->height - (sf->image_y<<1));
    //mev.y = ((float)ev->y*100.0)/(float)f->widget->height;
    mev.button = ev->button;
    mev.event = MOUSE_RELEASE;
    memcpy(buf, &type, sizeof(int));
    memcpy(&buf[sizeof(int)], &mev, sizeof(MouseEvent));
    pthread_mutex_lock(&sf->mutex);
    if(sf->sh != NULL) shark_send(sf->sh, buf, sizeof(int)+sizeof(MouseEvent));
    pthread_mutex_unlock(&sf->mutex);
}

void KeyDown(Form *f, KeyEventArgs *ev){
    KeyEvent kev;
    RCServerForm *sf = (RCServerForm*)f->obj;
    int type = EVENT_TYPE_KEY;
    char buf[SHARK_BUFFSIZE] = {0};
    kev.event = KEY_PRESS;
    kev.keySym = ev->ksym;
    memcpy(buf, &type, sizeof(int));
    memcpy(&buf[sizeof(int)], &kev, sizeof(KeyEvent));
    pthread_mutex_lock(&sf->mutex);
    if(sf->sh != NULL) shark_send(sf->sh, buf, sizeof(int)+sizeof(KeyEvent));
    pthread_mutex_unlock(&sf->mutex);
    if(ev->ksym == XK_Control_L){
        ctrlPressed = 1;
        return;
    }
    if((ev->keyChar == 'f' || ev->keyChar == 'а') && ctrlPressed){
        if(f->mode == FULLSCREEN_MODE){
            Form_SetWindowMode(f, WINDOW_MODE);
#if defined(OC2K1x)
            //T_Event_Emit(D_TERM_EVENT_HEADLINE_HIDE, NULL, 0);
            SS_Send0(PI_GetOurKto(), SS_GetID("D_HEADLINE_SHOW_SIGNAL"));
            sf->titleVisible = 1;
#endif
        }
        else{
            Form_SetWindowMode(f, FULLSCREEN_MODE);
#if defined(OC2K1x)
            //T_Event_Emit(D_TERM_EVENT_HEADLINE_HIDE, NULL, 1);
            SS_Send0(PI_GetOurKto(), SS_GetID("D_HEADLINE_HIDE_SIGNAL"));
            sf->titleVisible = 0;
#endif
        }
    }
    else if(ctrlPressed && (ev->keyChar == 'a' || ev->keyChar == 'ф')){
        if(sf->isFullWindow){
            sf->isFullWindow = 0;
            printf("Move x - %d, y - %d\n", sf->fullx, sf->fully);
            //Form_Move(f, sf->fullx, sf->fully);
            Form_Resize(f, sf->fullwidth, sf->fullheight);
            XResizeWindow(f->widget->display, f->widget->window, sf->fullwidth, sf->fullheight);
            Form_Move(f, sf->fullx, sf->fully);
        }
        else{
            sf->fullx = f->widget->x;
            sf->fully = f->widget->y;
            sf->fullwidth = f->widget->width;
            sf->fullheight = f->widget->height;
            sf->isFullWindow = 1;
            Form_Move(f, 0, 0);
            Form_Resize(f, sf->client_width, sf->client_height);
            XResizeWindow(f->widget->display, f->widget->window, sf->client_width, sf->client_height);
        }
    }
}

void KeyUp(Form *f, KeyEventArgs *ev){
    KeyEvent kev;
    RCServerForm *sf = (RCServerForm*)f->obj;
    int type = EVENT_TYPE_KEY;
    char buf[SHARK_BUFFSIZE] = {0};
    kev.event = KEY_RELEASE;
    kev.keySym = ev->ksym;
    memcpy(buf, &type, sizeof(int));
    memcpy(&buf[sizeof(int)], &kev, sizeof(KeyEvent));
    pthread_mutex_lock(&sf->mutex);
    if(sf->sh != NULL) shark_send(sf->sh, buf, sizeof(int)+sizeof(KeyEvent));
    pthread_mutex_unlock(&sf->mutex);
    if(ev->ksym == XK_Control_L){
        ctrlPressed = 0;
    }
}

void ResizeEvent(Form *f, ResizeEventArgs *ev){
    RCServerForm *sf = (RCServerForm*)f->obj;
    sf->resize_time = time_get();
    sf->resize_flag = 1;
    //pthread_mutex_lock(&sf->mutex);
    //RCServerForm_SendWindowSize(sf, ev->width, ev->height);
    //pthread_mutex_unlock(&sf->mutex);
}

int GetApPalete(unsigned char r, unsigned char g, unsigned char b, RCServerForm *sf){
    int i, res, index = 0, min = INT32_MAX;
    for(i = 0; i < 256; i++){
        res = 30 * (sf->serv_pal_s[i][0] - r)*(sf->serv_pal_s[i][0]-r) + 59*(sf->serv_pal_s[i][1]-g)*(sf->serv_pal_s[i][1]-g) + 11*(sf->serv_pal_s[i][2] - b)*(sf->serv_pal_s[i][2] - b);
        if(res < min){
            index = i;
            min = res;
        }
    }
    return index;
}

void MakePalete(RCServerForm *sf){
    int i;
    for(i = 0; i < 256; i++){
        sf->serv_pal_s[i][0] = sf->serv_pal[i][0]>>8;
        sf->serv_pal_s[i][1] = sf->serv_pal[i][1]>>8;
        sf->serv_pal_s[i][2] = sf->serv_pal[i][2]>>8;
    }
    for(i = 0; i < 256; i++){
        sf->ap_pal[i] = GetApPalete(sf->pal[i][0]>>8, sf->pal[i][1]>>8, sf->pal[i][2]>>8, sf);
    }
}

void MakeBigPalete(RCServerForm *sf){
    //int r, g, b;
    int i;
    if(sf->big_pal == NULL){
        sf->big_pal = (unsigned int*)malloc(sizeof(unsigned int)*BIG_PAL_SIZE);
    }
    //    for(r = 0; r < 256; r++){
    //        for(g = 0; g < 256; g++){
    //            for(b = 0; b < 256; b++){
    //                sf->big_pal[(r<<16)|(g<<8)|b] = GetApPalete(r, g, b, sf);
    //            }
    //        }
    //    }
    //    for(r = 0; r < 64; r++){
    //        for(g = 0; g < 64; g++){
    //            for(b = 0; b < 64; b++){
    //                sf->big_pal[(r<<12)|(g<<6)|b] = GetApPalete(r<<2, g<<2, b<<2, sf);
    //            }
    //        }
    //    }

    for(i = 0; i < BIG_PAL_SIZE; i++){
        sf->big_pal[i] = GetApPalete((i>>11)<<3,((i>>5)&0x3f)<<2, (i&0x1f)<<3, sf);
    }
}

RCServerForm *RCServerForm_newRCServerForm(int client_addr, int client_port, int server_port, int client_depth, int client_width, int client_height,
                                           unsigned short pal[][3], unsigned short serv_pal[][3], char *hostIp, char *chname, char *s_addr){
    RCServerForm *sf = (RCServerForm*)malloc(sizeof(RCServerForm));
    if(sf == NULL) return NULL;
    sf->client_addr = client_addr;
    sf->client_port = client_port;
    sf->server_port = server_port;
    sf->client_depth = client_depth;
    sf->client_width = client_width;
    sf->client_height = client_height;
    sf->client_saddr = s_addr;
    sf->isFullWindow = 0;
    sf->imageData = NULL;
    sf->fullwidth = sf->fullheight = sf->x = sf->y;
    sf->active = 0;
    sf->big_pal = NULL;
    sf->qu = NULL;
    sf->sw_time = 0;
    memcpy(sf->pal, pal, sizeof(sf->pal));
    memcpy(sf->serv_pal, serv_pal, sizeof(sf->serv_pal));
    //if(client_depth == 8 && Color_GetDepth() == 8) MakePalete(sf);
    //if((client_depth == 8 || client_depth == 24 || client_depth == 16) && Color_GetDepth() == 8) MakeBigPalete(sf);
    if(Color_GetDepth() == 8){
        MakePalete(sf);
        MakeBigPalete(sf);
    }
    sf->sh = NULL;
    sf->Close = NULL;
#if defined(OC2K1x)
    sf->titleVisible = 1;
#else
    sf->titleVisible = 0;
#endif
    //sf->title = hostIp;
    sf->title = calloc(120, 1);
    sprintf(sf->title, "%s (%s)", chname, hostIp);
    memcpy(sf->chName, chname, sizeof(sf->chName));
    sf->disconnected = 1;
    sf->crHover = 0;
    sf->image_x = 0;
    sf->image_y = 0;
    sf->resize_flag = 0;
    return sf;
}

void RCServerForm_SetClientPassive(RCServerForm *sf){
    char buf[SHARK_BUFFSIZE] = {0};
    int com = C_SET_PASSIVE_STATE;
    int size = sizeof(int);
    memcpy(buf, &com, size);
    pthread_mutex_lock(&sf->mutex);
    shark_send(sf->sh, buf, size);
    pthread_mutex_unlock(&sf->mutex);
}

void RCServerForm_Dispose(RCServerForm *sf){
    if(sf == NULL) return;
    sf->active = 0;
    pthread_join(sf->im_th, NULL);
    pthread_join(sf->pack_th, NULL);
    if(sf->sh != NULL){
        RCServerForm_SetClientPassive(sf);
        shark_close(sf->sh);
    }
    pthread_mutex_lock(&sf->mutex);
    if(sf->imageData){
        free(sf->imageData);
        sf->imageData = NULL;
    }
    pthread_mutex_unlock(&sf->mutex);
    pthread_mutex_destroy(&sf->mutex);
    if(sf->Close != NULL) sf->Close(sf->client_addr);
    if(sf->big_pal != NULL) free(sf->big_pal);
    free(sf);
    free(sf->title);
    sf = NULL;
}

void RCServerForm_Start(RCServerForm *sf, int x, int y, int width, int height){
    sf->sh = shark_init(sf->client_port, sf->server_port, sf->client_saddr, NULL);
    if(sf->sh == NULL) return;
    thread_pause(100);
    sf->active = 1;
    sf->tmp_width = width;
    sf->tmp_height = height;
    sf->image_y = 0;
    sf->x = x;
    sf->y = y;
#if defined(OC2K1x)
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = 100;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&sf->m_th, &attr, MainThread, (void*)sf);
#else
    pthread_create(&sf->m_th, NULL, MainThread, (void*)sf);
#endif
}

void RCServerForm_Stop(RCServerForm *sf){
    RCServerForm_Dispose(sf);
}

void *MainThread(void *arg){
    RCServerForm *sf = (RCServerForm*)arg;
    if(sf->tmp_width == 0 || sf->tmp_height == 0) {
        sf->tmp_width = SF_WIDTH;
        sf->tmp_height = SF_HEIGHT;
    }
    sf->f = Form_newForm(sf->x, sf->y, sf->tmp_width, sf->tmp_height, MAIN_FORM, NULL);
    //sf->f = Form_newForm(0, 0, sf->client_width, sf->client_height, MAIN_FORM, NULL);
    if(sf->f == NULL){
        free(sf);
        sf = NULL;
        return NULL;
    }
    //if(sf->tmp_width != SF_WIDTH && sf->tmp_height != SF_HEIGHT) sf->f->title_hide = 1;
    sf->f->im = NULL;
    sf->f->Paint = RCServerForm_Paint;
    sf->f->MouseMove = MouseMove;
    sf->f->MousePress = MousePress;
    sf->f->MouseRelease = MouseRelease;
    sf->f->KeyDown = KeyDown;
    sf->f->KeyUp = KeyUp;
    sf->f->ResizeEvent = ResizeEvent;
    sf->f->Close = RCServerForm_Close;
    sf->f->MouseEnter = RCServerForm_Enter;
    sf->f->MouseLeave = RCServerForm_Leave;
    sf->f->obj = (void*)sf;
    //int res = XReadBitmapFile(sf->f->widget->display, sf->f->widget->window, "cursor.bmp", &sf->curp_width, &sf->curp_height, &sf->curPix, &sf->curp_x, &sf->curp_y);
    char title[32] = {0};
    sprintf(title, "192.9.2.%d", sf->client_addr);
    //Form_SetTitle(sf->f, title);
    Form_SetTitle(sf->f, sf->title);
    Form_SetSizePolicy(sf->f, 200, 200, sf->client_width<<1, sf->client_height<<1);
    pthread_mutex_init(&sf->mutex, NULL);

#if defined(OC2K1x)
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = 110;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&sf->im_th, &attr, ImageThread, (void*)sf);
    RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);
    pthread_attr_t attr_p;
    pthread_attr_init(&attr_p);
    pthread_attr_setschedpolicy(&attr_p, SCHED_OTHER);
    pthread_attr_getschedparam(&attr_p, &param);
    param.sched_priority = 100;
    pthread_attr_setschedparam(&attr_p, &param);
    pthread_create(&sf->pack_th, &attr_p, RCServerForm_PackImage, (void*)sf);
#else

    //    pthread_attr_t attr;
    //    struct sched_param param;
    //    pthread_attr_init(&attr);
    //    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    //    pthread_attr_getschedparam(&attr, &param);
    //    param.__sched_priority = 110;
    //    pthread_attr_setschedparam(&attr, &param);

    pthread_create(&sf->im_th, NULL, ImageThread, (void*)sf);
    RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);

    //    pthread_attr_t attr_p;
    //    pthread_attr_init(&attr_p);
    //    pthread_attr_setschedpolicy(&attr_p, SCHED_OTHER);
    //    pthread_attr_getschedparam(&attr_p, &param);
    //    param.__sched_priority = 90;
    //    pthread_attr_setschedparam(&attr_p, &param);
    pthread_create(&sf->pack_th, NULL, RCServerForm_PackImage, (void*)sf);
#endif
    //pthread_create(&sf->im_th, NULL, ImageThread, (void*)sf);
    Form_Show(sf->f);
    //    pthread_attr_destroy(&attr);
    //    pthread_attr_destroy(&attr_p);
    //RCServerForm_Dispose(sf);
    return NULL;
}

void *ImageThread(void *arg){
    RCServerForm *sf = (RCServerForm*)arg;
    char *buf;
    char *imageBuf = NULL;
    int size, numpack, seek, cur_pack, size_rcv, image_size;
    ImageInfo imInfo;
    imInfo.rle = 0;
    int count = 0;
    int com = 0;
    //unsigned long tt;
    unsigned int fps = 0;
    int lost_count = 0;
    unsigned long time_speed;
    unsigned int byte_received = 0;
    unsigned long stat_bytes_recv = 0;
    char flag = 0;
    unsigned long cursor_time = time_get();
    Event fev;
    ImagePackInfo *imPackInfo;
    Events evs;
    size = numpack = seek = cur_pack = size_rcv = image_size = 0;
    time_speed = time_get();
    while(sf->active){
        buf = shark_read(sf->sh, 0, &size);
        byte_received += size;
        stat_bytes_recv += size;
        if(size < (int)sizeof(int)){
            if(!sf->resize_flag || sf->resize_time - time_get() > RESIZE_TIME) {
                sf->resize_flag = 0;
                RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);
            }
            //RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);
            count++;
            sf->disconnected = 1;
            fev.type = EV_FORM_PAINT;
            fev.func = NULL;
            pthread_mutex_lock(&sf->f->evs_mutex);
            EventStack_Push(&sf->f->evStack, fev);
            pthread_mutex_unlock(&sf->f->evs_mutex);
            if(count > 4){
                if(size_rcv > 0){
                    if(imageBuf != NULL) free(imageBuf);
                    imageBuf = NULL;
                    //free(old);
                }
                break;//return NULL;
            }
            continue;
        }
        count = 0;
        if(size == (int)sizeof(int)) continue;
        else if(size == (sizeof(sf->pal) + sizeof(int))){
            memcpy(&com, buf, sizeof(int));
            if(com == C_SET_COLORMAP){
                memcpy(&sf->pal, &buf[sizeof(int)], sizeof(sf->pal));
                MakePalete(sf);
                MakeBigPalete(sf);
                continue;
            }
        }
        else if(size == sizeof(int)*3){
            memcpy(&com, buf, sizeof(int));
            if(com == C_SET_MOUSE_COORD){
                float x, y;
                memcpy(&x, &buf[sizeof(int)], sizeof(int));
                memcpy(&y, &buf[sizeof(int)*2], sizeof(int));
                if(sf->f->im != NULL){
                    sf->client_mouse_x = (x/100.0)*((float)sf->f->widget->width - (sf->f->widget->width-sf->f->im->width)) + sf->image_x;
                    //if(sf->titleVisible) sf->client_mouse_y = (y/100.0)*((float)sf->f->widget->height  - ((sf->image_y<<1)+TITLE_HEIGHT)) + TITLE_HEIGHT + sf->image_y;
                    if(sf->titleVisible) sf->client_mouse_y = (y/100.0)*((float)sf->f->widget->height - (sf->image_y+(sf->image_y-TITLE_HEIGHT))) + sf->image_y;
                    else sf->client_mouse_y = (y/100.0)*((float)sf->f->widget->height - (sf->image_y<<1));
                }
                else{
                    sf->client_mouse_x = (x/100.0)*(float)sf->f->widget->width;
                    sf->client_mouse_y = (y/100.0)*((float)sf->f->widget->height + (sf->image_y<<1));
                }
                fev.type = EV_FORM_PAINT;
                fev.func = NULL;
                if(time_get() - cursor_time > 40){
                    pthread_mutex_lock(&sf->f->evs_mutex);
                    EventStack_Push(&sf->f->evStack, fev);
                    pthread_mutex_unlock(&sf->f->evs_mutex);
                    cursor_time = time_get();
                }
                RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);
                continue;
            }
        }
        memcpy(&numpack, buf, sizeof(int));
        if(numpack == 0){
            //tt = time_get();
            if(size_rcv != image_size){
                if(imageBuf != NULL) free(imageBuf);
                imageBuf = NULL;
                size_rcv = image_size = 0;
                if(!imInfo.rle){
                    flag = 1;
                    RCServerForm_SendGetFullImage(sf);
                }
                //printf("Pack lost!\n");
            }
            memcpy(&imInfo, &buf[sizeof(int)], sizeof(ImageInfo));
            if(flag && imInfo.rle) continue;
            else if(!imInfo.rle) flag = 0;
            image_size = imInfo.size;
            if(image_size < 1) continue;
            imageBuf = (char*)malloc(sizeof(char)*image_size);
            size_rcv = size - (sizeof(int)+sizeof(ImageInfo));
            if(size_rcv < 1){
                size_rcv = 0;
                free(imageBuf);
                imageBuf = NULL;
            }
            seek = size_rcv;
            memcpy(imageBuf, &buf[sizeof(int)+sizeof(ImageInfo)], size_rcv);
            cur_pack = 1;
            if(size_rcv == image_size){
                sf->disconnected = 0;
                imPackInfo = (ImagePackInfo*)malloc(sizeof(ImagePackInfo));
                imPackInfo->width = imInfo.width;
                imPackInfo->height = imInfo.height;
                imPackInfo->depth = imInfo.depth;
                imPackInfo->rle = imInfo.rle;
                imPackInfo->size = imInfo.size;
                imPackInfo->buf = imageBuf;
                imPackInfo->cur_height = imInfo.cur_height;
                evs.type = 0;
                evs.event = (void*)imPackInfo;
                EventsQueue_Push(&sf->qu, evs);
                size_rcv = image_size = cur_pack = seek = 0;
                imageBuf = NULL;
                fps++;
                if(!sf->resize_flag || sf->resize_time - time_get() > RESIZE_TIME) {
                    sf->resize_flag = 0;
                    RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);
                }
                cursor_time = time_get();
            }
        }
        else if(numpack != cur_pack){
            if(imageBuf != NULL) free(imageBuf);
            imageBuf = NULL;
            size_rcv = image_size = cur_pack = 0;
            lost_count++;
            if(!imInfo.rle){
                flag = 1;
                RCServerForm_SendGetFullImage(sf);
            }
            //printf("lost count - %d\n\n", lost_count);
        }
        else{
            size_rcv += (size - sizeof(int));
            if(size_rcv > image_size){
                lost_count++;
                if(imageBuf != NULL) free(imageBuf);
                size_rcv = image_size = seek = cur_pack = 0;
                if(!imInfo.rle){
                    flag = 1;
                    RCServerForm_SendGetFullImage(sf);
                }
                //printf("Pack lost - %d\n", lost_count);
            }
            else{
                memcpy(&imageBuf[seek], &buf[sizeof(int)], size-sizeof(int));
                if(size_rcv == image_size){
                    //tt = time_get();
                    sf->disconnected = 0;
                    imPackInfo = (ImagePackInfo*)malloc(sizeof(ImagePackInfo));
                    imPackInfo->width = imInfo.width;
                    imPackInfo->height = imInfo.height;
                    imPackInfo->depth = imInfo.depth;
                    imPackInfo->rle = imInfo.rle;
                    imPackInfo->size = imInfo.size;
                    imPackInfo->buf = imageBuf;
                    imPackInfo->cur_height = imInfo.cur_height;
                    evs.type = 0;
                    evs.event = (void*)imPackInfo;
                    EventsQueue_Push(&sf->qu, evs);
                    fps++;
                    //RCServerForm_PackImage(sf, imageBuf, &imInfo, &old);
                    //printf("Recv count - %d\n", ++a);
                    //tt = time_get() - tt;
                    //printf("Time pack image = %d\n\n", (int)tt);
                    size_rcv = image_size = cur_pack = seek = 0;
                    imageBuf = NULL;
                    if(!sf->resize_flag || sf->resize_time - time_get() > RESIZE_TIME) {
                        sf->resize_flag = 0;
                        RCServerForm_SendWindowSize(sf, sf->f->widget->width, sf->f->widget->height);
                    }
                    cursor_time = time_get();
                    continue;
                }
                seek = size_rcv;
                cur_pack++;
            }
        }
        if(time_get() - time_speed >= 1000){
            char sbs[30] = {0};
            if(stat_bytes_recv > 1024*1024*1024)	sprintf(sbs, "%lu Gb", stat_bytes_recv/(1024*1024*1024));
            else if(stat_bytes_recv > 1024*1024)	sprintf(sbs, "%lu Mb", stat_bytes_recv/(1024*1024));
            else if(stat_bytes_recv > 1024) sprintf(sbs, "%lu Kb", stat_bytes_recv/1024);
            else sprintf(sbs, "%lu Byte", stat_bytes_recv);
            //printf("Bytes received - %d Kb/s\n", byte_received/1024);e
            printf("\n*****************************\n"
                   "Client '%s'\n"
                   "Speed - %d Kb/s\n"
                   "Fps - %d\n"
                   "Bytes received - %s\n"
                   "*****************************\n", sf->chName, byte_received/1024, fps, sbs);
            fps = 0;
            byte_received = 0;
            time_speed = time_get();
        }
    }
    //free(old);
    Event ev;
    ev.type = EV_CLOSE_WINDOW;
    ev.func = NULL;
    ev.arg = NULL;
    pthread_mutex_lock(&sf->f->evs_mutex);
    EventStack_Push(&sf->f->evStack, ev);
    pthread_mutex_unlock(&sf->f->evs_mutex);
    return NULL;
}

void RCServerForm_ChangeIndex(char *src, int size, RCServerForm *sf){
    int i;
    for(i = 0; i < size; i++){
        src[i] = sf->ap_pal[(unsigned char)src[i]];
    }
}

char *RCServerForm_RedepthImage8x8(char *src, int size, RCServerForm *sf){
    char *dst = (char*)malloc(sizeof(char)*size/4);
    int i, j, val;
    unsigned int *it = (unsigned int*)src;
    for(i = 0, j = 0; i < size; i+=4, it++){
        val = (((*it>>19)&0x1f)<<11) | (((*it>>10)&0x3f)<<5) | ((*it>>3)&0x1f);
        dst[j++] = sf->big_pal[val];
    }
    //    for(i = 0, j = 0; i < size; i+=4, it++){
    //        dst[j++] = sf->big_pal[(((*it>>16)&0xcf)<<12) | (((*it>>8)&0xcf)<<6) | (*it&0xcf)];
    //    }
    //    for(i = 0, j = 0; i < size; i+=4){
    //        dst[j++] = sf->big_pal[*it++];
    //    }
    /*for(i = 0, j = 0; i < size; i+=3, j++){
        dst[j] = GetApPalete(src[i+2], src[i+1], src[i], sf);
    }*/
    return dst;
}

void *RCServerForm_PackImage(void *arg){
    RCServerForm *sf = (RCServerForm*)arg;
    char *old = NULL;
    Events evs;
    Event ev;
    int n, tmp;
    ImagePackInfo *imInfo;
    int size;
    unsigned long dst_size, src_size;
    char *imageBuf = NULL;
    char *tmpBuf = NULL;
    int depth = Color_GetDepth();
    while(sf->active){
        while(EventsQueue_Pop(&sf->qu, &evs) > 0){
            imInfo = (ImagePackInfo*)evs.event;
            if(imInfo->depth == 24) n = 4;
            else if(imInfo->depth == 16) n = 2;
            else n = 1;
            if(n == 1 && (imInfo->width != sf->client_width || imInfo->height != sf->client_height)) n = 4;
            size = imInfo->width * imInfo->height * n;
            imageBuf = (char*)malloc(sizeof(char)*size);
            src_size = imInfo->size;
            dst_size = size;
            RCServerForm_DecompressImage(imInfo->buf, src_size, imageBuf, &dst_size);
            free(imInfo->buf);
            if(dst_size == 0){
                free(imageBuf);
                break;
            }
            if(imInfo->rle){
                tmpBuf = imageBuf;
                src_size = dst_size;
                imageBuf = (char*)malloc(sizeof(char)*size);
                tmp = size;
                RCServerForm_RleDecompress3(&old, tmpBuf, src_size, imageBuf, &tmp, n);
                dst_size = tmp;
                free(tmpBuf);
                if(dst_size < 1){
                    free(imageBuf);
                    break;
                }
            }
            else{
                if(old != NULL) free(old);
                old = (char*)malloc(sizeof(char)*dst_size);
                memcpy(old, imageBuf, dst_size);
            }
            if(imInfo->depth != depth){
                if(imInfo->depth == 24 && depth == 16){
                    //tmpBuf = RCServerForm_RedepthImage24x16(imageBuf, dst_size);
                    tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 24, 16, NULL, NULL);
                }
                else if(imInfo->depth == 8 && depth == 24){
                    if(imInfo->width != sf->client_width || imInfo->height != sf->client_height){
                        tmpBuf = RCServerForm_RedepthImage8x24_1(imageBuf, dst_size);
                    }
                    else {
                        //tmpBuf = RCServerForm_RedepthImage8x24(imageBuf, dst_size, sf);
                        tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 8, 24, sf->pal, NULL);
                    }
                }
                else if(imInfo->depth == 8 && depth == 16){
                    if(imInfo->width != sf->client_width || imInfo->height != sf->client_height)
                        tmpBuf = RCServerForm_RedepthImage8x16_1(imageBuf, dst_size);
                    else
                        //tmpBuf = RCServerForm_RedepthImage8x16(imageBuf, dst_size, sf);
                        tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 8, 16, sf->pal, NULL);
                }
                else if(imInfo->depth == 24 && depth == 8){
                    //tmpBuf = RCServerForm_RedepthImage24x8(imageBuf, dst_size, sf);
                    tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 24, 8, NULL, sf->big_pal);
                }
                else if(imInfo->depth == 16 && depth == 8){
                    //tmpBuf = RCServerForm_RedepthImage16x8(imageBuf, dst_size, sf);
                    tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 16, 8, NULL, sf->big_pal);
                }
                else{
                    //tmpBuf = RCServerForm_RedepthImage16x24(imageBuf, dst_size);
                    tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 16, 24, NULL, NULL);
                }
                /*if(imInfo->depth == 8 && depth == 24){
                    if(imInfo->width == sf->client_width && imInfo->height == sf->client_height){
                        free(imageBuf);
                        imageBuf = tmpBuf;
                    }
                }*/
                //else{
                free(imageBuf);
                imageBuf = tmpBuf;
                //}
            }
            else if(imInfo->depth == 8 && depth == 8 && (imInfo->width != sf->client_width || imInfo->height != sf->client_height)){
                //tmpBuf = RCServerForm_RedepthImage8x8(imageBuf, dst_size, sf);
                tmpBuf = Graphics_RedepthImage(imageBuf, dst_size, 8, 8, NULL, sf->big_pal);
                free(imageBuf);
                imageBuf = tmpBuf;
            }
            else if(imInfo->depth == 8 && depth == 8 && memcmp(sf->serv_pal, sf->pal, sizeof(sf->pal)) != 0){
                RCServerForm_ChangeIndex(imageBuf, dst_size, sf);
            }
            ev.param = imInfo->width;
            ev.param1 = imInfo->cur_height;//imInfo->height;

            pthread_mutex_lock(&sf->mutex);
            sf->image_width = imInfo->width;
            sf->image_height = imInfo->cur_height;
            sf->n = n;
            if(sf->imageData) free(sf->imageData);
            sf->imageData = malloc(imInfo->width * imInfo->cur_height * n);
            memcpy(sf->imageData, imageBuf, imInfo->width*imInfo->cur_height * n);
            pthread_mutex_unlock(&sf->mutex);

            ev.arg = imageBuf;
            ev.type = EV_SET_IMAGE;
            ev.func = NULL;
            pthread_mutex_lock(&sf->f->evs_mutex);
            EventStack_Push(&sf->f->evStack, ev);
            pthread_mutex_unlock(&sf->f->evs_mutex);
            free(imInfo);
        }
        thread_pause(1);
    }
    if(old != NULL) free(old);
    return NULL;
}

char *RCServerForm_GetImageData(RCServerForm *sf, int *width, int *height, int *n){
    char *buf;
    *width = *height = *n = 0;
    if(!sf->imageData) return NULL;
    pthread_mutex_lock(&sf->mutex);
    buf = malloc(sf->image_width * sf->image_height * sf->n);
    *width = sf->image_width;
    *height = sf->image_height;
    *n = sf->n;
    memcpy(buf, sf->imageData, sf->image_width * sf->image_height * sf->n);
    pthread_mutex_unlock(&sf->mutex);
    return buf;
}
