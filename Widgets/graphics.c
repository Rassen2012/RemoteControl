#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "widget.h"
#include "graphics.h"


#define FONT_SIZE_10 "-*-*-medium-r-normal--10-*-*-*-*-*-koi8-r"
#define FONT_SIZE_13 "-*-*-medium-r-normal--13-*-*-*-*-*-koi8-r"
#define FONT_SIZE_14 "-*-*-medium-r-normal--14-*-*-*-*-*-koi8-r"
#define FONT_SIZE_15 "-*-*-medium-r-normal--15-*-*-*-*-*-koi8-r"
#define FONT_SIZE_18 "-*-*-medium-r-normal--18-*-*-*-*-*-koi8-r"
#define FONT_SIZE_20 "-*-*-medium-r-normal--20-*-*-*-*-*-koi8-r"

Graphics *Graphics_newGraphics(S_Widget *widget){
    Graphics *g;
    if(widget == NULL) return NULL;
    g = (Graphics*)malloc(sizeof(Graphics));
    if(g == NULL) return NULL;
    g->widget = widget;
    g->context = XCreatePixmap(g->widget->display, g->widget->window, g->widget->width, g->widget->height,
                               Color_GetDepth());
    g->fontInfo = NULL;
    g->gc = XCreateGC(g->widget->display, g->widget->window, 0, NULL);
    Graphics_SetFont(g, NULL);
    g->cur_color = BLACK_COLOR(g->widget->display);
    Graphics_SetColor(g, g->cur_color);
    return g;
}

void Graphics_Dispose(Graphics *g){
    if(g == NULL) return;
    XFreeFont(g->widget->display, g->fontInfo);
    XFreePixmap(g->widget->display, g->context);
    XFreeGC(g->widget->display, g->gc);
    free(g);
}

void Graphics_Resize(Graphics *g, ResizeEventArgs *ev){
    Pixmap pix = XCreatePixmap(g->widget->display, g->widget->window, ev->width, ev->height, Color_GetDepth());
    //Color cur = g->cur_color;
    //Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
    //XFillRectangle(g->widget->display, pix, g->gc, 0, 0, ev->width, ev->height);
    XCopyArea(g->widget->display, g->context, pix, g->gc, 0, 0, ev->old_width, ev->old_height, 0, 0);
    if(ev->width > ev->old_width || ev->height > ev->old_height){
        Color cur = g->cur_color;
        Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
        if(ev->width > ev->old_width){
           XFillRectangle(g->widget->display, pix, g->gc, ev->old_width, 0, ev->width - ev->old_width, ev->height);
        }
        else{
            XFillRectangle(g->widget->display, pix, g->gc, 0, ev->old_height, ev->width, ev->height - ev->old_height);
        }
        Graphics_SetColor(g, cur);
    }
    XFreePixmap(g->widget->display, g->context);
    g->context = pix;
    //Graphics_SetColor(g, cur);
    //printf("Begin resize - %dx%d\n", width, height);
    //g->context = XCreatePixmap(g->widget->display, g->widget->window, width, height, Color_GetDepth());
    //printf("End resize\n");
}

void Graphics_SetFont(Graphics *g, char *font){
    if(g->fontInfo != NULL){
        XFreeFont(g->widget->display, g->fontInfo);
    }
    if(font != NULL){
        g->fontInfo = XLoadQueryFont(g->widget->display, font);
    }
    else{
        g->fontInfo = XLoadQueryFont(g->widget->display, FONT_SIZE_14);
    }
    XSetFont(g->widget->display, g->gc, g->fontInfo->fid);
}

void Graphics_SetFontSize(Graphics *g, int size){
    switch(size){
    case 10:
        Graphics_SetFont(g, FONT_SIZE_10);
        break;
    case 12:
    case 13:
        Graphics_SetFont(g, FONT_SIZE_13);
        break;
    case 14:
        Graphics_SetFont(g, FONT_SIZE_14);
        break;
    case 15:
        Graphics_SetFont(g, FONT_SIZE_15);
        break;
    case 16:
    case 18:
        Graphics_SetFont(g, FONT_SIZE_18);
        break;
    case 20:
        Graphics_SetFont(g, FONT_SIZE_20);
        break;
    default:
        Graphics_SetFont(g, FONT_SIZE_14);
        break;
    }
}

void Graphics_SetColor(Graphics *g, Color c){
    g->cur_color = c;
    XSetForeground(g->widget->display, g->gc, c.l);
}

void Graphics_SetLineWidth(Graphics *g, int width){
    XSetLineAttributes(g->widget->display, g->gc, width, LineSolid, CapRound, JoinRound);
}

void Graphics_BeginPaint(Graphics *g, PaintEventArgs *ev){
    g->ev = ev;
    //g->gc = XCreateGC(g->widget->display, g->widget->window, 0, NULL);
    if(ev == NULL) XCopyArea(g->widget->display, g->widget->window, g->context, g->gc, 0, 0, g->widget->width, g->widget->height, 0, 0);
    else XCopyArea(g->widget->display, g->widget->window, g->context, g->gc, ev->x, ev->y, ev->width, ev->height, ev->x, ev->y);
}

void Graphics_EndPaint(Graphics *g){
    if(g->ev == NULL) XCopyArea(g->widget->display, g->context, g->widget->window, g->gc, 0, 0, g->widget->width, g->widget->height, 0, 0);
    else XCopyArea(g->widget->display, g->context, g->widget->window, g->gc, g->ev->x, g->ev->y, g->ev->width, g->ev->height,g->ev->x, g->ev->y);
    //XFreeGC(g->widget->display, g->gc);
    //XFlush(g->widget->display);
}

void Graphics_DrawPoint(Graphics *g, int x, int y){
    XDrawPoint(g->widget->display, g->context, g->gc, x, y);
}

void Graphics_DrawLine(Graphics *g, int x1, int y1, int x2, int y2){
    XDrawLine(g->widget->display, g->context, g->gc, x1, y1, x2, y2);
}

void Graphics_DrawRectangle(Graphics *g, int x, int y, int width, int height){
    XDrawRectangle(g->widget->display, g->context, g->gc, x, y, width, height);
}

void Graphics_DrawEllips(Graphics *g, int x_center, int y_center, int width, int height){
    XDrawArc(g->widget->display, g->context, g->gc, x_center - (width>>1), y_center - (height>>1), width, height, 0, 360*64);
}

void Graphics_DrawText(Graphics *g, int x, int y, const char *text){
    XDrawString(g->widget->display, g->context, g->gc, x, y, text, strlen(text));
}

void Graphics_DrawImage(Graphics *g, Pixmap image, int x, int y, int width, int height, int xd, int yd){
    XCopyArea(g->widget->display, image, g->context, g->gc, x, y, width, height, xd, yd);
}

void Graphics_FillEllips(Graphics *g, int x_center, int y_center, int width, int height){
    XFillArc(g->widget->display, g->context, g->gc, x_center - (width>>1), y_center - (height>>1), width, height, 0, 360*64);
}

void Graphics_FillRectangle(Graphics *g, int x, int y, int width, int height){
    XFillRectangle(g->widget->display, g->context, g->gc, x, y, width, height);
}

void Graphics_FillPolygon(Graphics *g, XPoint *points, int npoints){
    XFillPolygon(g->widget->display, g->context, g->gc, points, npoints, Complex, CoordModeOrigin); // Comprex ???? Not quick!
}

void Graphics_FillTriangle(Graphics *g, XPoint *points){ // max 3 points
    XFillPolygon(g->widget->display, g->context, g->gc, points, 3, Convex, CoordModeOrigin);
}

void Graphics_DrawSText(Graphics *g, char *text){
    XTextItem it;
    it.chars = text;
    it.delta = 0;
    it.font = g->fontInfo->fid;
    it.nchars = strlen(text);
    XDrawText(g->widget->display, g->context, g->gc, 20, 20, &it, 1);
}

char* Graphics_RedepthImage(char *src, int size, int src_depth, int dest_depth, unsigned int **pal, unsigned char *big_pal){
    switch(src_depth){
    case 8:
        switch(dest_depth){
        case 8:
        {
            char *dst = (char*)malloc(size/4);
            int i, j, val;
            unsigned int *it = (unsigned int*)src;
            for(i = 0, j = 0; i < size; i+=4, it++){
                val = (((*it>>19)&0x1f)<<11) | (((*it>>10)&0x3f)<<5) | ((*it>>3)&0x1f);
                dst[j++] = big_pal[val];
            }
            return dst;
        }
        case 16:
        {
            size_t step = 2;
            char *dst = (char*)malloc(size*step);
            int i, j;
            unsigned int c;
            unsigned char r, g, b;
            for(i = 0, j = 0; i < size; i++, j += step){
                r = pal[(unsigned char)src[i]][0]>>8;
                g = pal[(unsigned char)src[i]][1]>>8;
                b = pal[(unsigned char)src[i]][2]>>8;
                Color_GetColor(r, g, b);
                memcpy(&dst[j], &c, step);
            }
            return dst;
        }
        case 24:
        {
            char *dst = (char*)malloc(size*4);
            int i, j;
            unsigned int c;
            unsigned char r, g, b;
            size_t step = 4;
            for(i = 0, j = 0; i < size; i++, j+=step){
                r = pal[(unsigned char)src[i]][0]>>8;
                g = pal[(unsigned char)src[i]][1]>>8;
                b = pal[(unsigned char)src[i]][2]>>8;
                c = (r<<16)|(g<<8)|b;
                memcpy(&dst[j], &c, step);
            }
            return dst;
        }
        }
        break;
    case 16:
        switch(dest_depth){
        case 8:
        {
            char *dst = (char*)malloc(size/2);
            int i, j;
            unsigned short *dsrc = (unsigned short*)src;
            for(i = 0, j = 0; i < size; i+=2){
                dst[j++] = big_pal[*dsrc++];
            }
            return dst;
        }
        case 24:
        {
            char *dst = (char*)malloc(size*2);
            int i;
            unsigned int c;
            unsigned short *it_i = (unsigned short*)src;
            unsigned int *it_d = (unsigned int*)dst;
            for(i = 0; i < size; i += 2){
                c = *it_i++;
                c = ((c>>8)&0xf8)<<16 | ((c>>3)&0xfc)<<8 | (c&0x1f)<<3;
                *it_d++ = c;
            }
            return dst;
        }
        }
        break;
    case 24:
        switch(dest_depth){
        case 8:
        {
            char *dst = (char*)malloc(size/4);
            int i, j, val;
            unsigned int *it = (unsigned int*)src;
            for(i = 0, j = 0; i < size; i+=4, it++){
                val = (((*it>>19)&0x1f)<<11) | (((*it>>10)&0x3f)<<5) | ((*it>>3)&0x1f);
                dst[j++] = big_pal[val];
            }
            return dst;
        }
        case 16:
        {
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
        }
        break;
    }
    return NULL;
}

void Graphics_RleDecompress(char **old, char *src, int src_size, char *dst, int *dst_size, int n){
    int i, s = 0, v = 0;
    unsigned char sym_count = 0;
    unsigned int cur = 0;
    unsigned char val = 0;
    char *t = *old;
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
                for(i = 0; i < sym_count; i++){
                    memcpy(&dst[v], &cur, step);
                    v+=step;
                }
                s+=step;
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
                for(i = 0; i < sym_count; i++){
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
    *dst_size = v;
}
