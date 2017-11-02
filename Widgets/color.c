#include <X11/X.h>
#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>
#include "color.h"

unsigned long ANDRES_RGB_Color(unsigned char r,unsigned char g,unsigned char b);

int col_depth = 0;
Colormap colormap;
Display *disp = NULL;

Color Color_GetColor(unsigned char r, unsigned char g, unsigned char b){
    Color c;
    XColor col;
    if(disp == NULL) disp = XOpenDisplay(NULL);
    c.r = r;
    c.g = g;
    c.b = b;
    col.red = r<<8;
    col.green = g<<8;
    col.blue = b<<8;
    if(col_depth == 24) c.l = (r<<16)|(g<<8)|b;
    else if(col_depth == 16) c.l = ((r&0xf8)<<8)|((g&0xfc)<<3)|((b>>3)&0x1f);
    else{
        c.l = col.pixel = (r<<16) | (g<<8) | b;
#ifdef SELF_COMPILE
        XAllocColor(disp, colormap, &col);
        c.l = col.pixel;
#else
        //XQueryColor(disp, colormap, &col);
        c.l=ANDRES_RGB_Color(r,g,b);
#endif
        c.r = col.red>>8;
        c.g = col.green>>8;
        c.b = col.blue>>8;
    }

//    switch(col_depth){
//    case 16:
//        c.l = ((r&0xf8)<<8)|((g&0xfc)<<3)|((b>>3)&0x1f);
//        break;
//    case 8:
//        break;
//    case 24:
//    default:
//        c.l = (r<<16)|(g<<8)|b;
//        break;
//    }
    return c;
}

Color Color_GetColor1(unsigned char r, unsigned char g, unsigned char b, Display *d){
    Color c;
    XColor col;
    c.r = r;
    c.g = g;
    c.b = b;
    col.red = r<<8;
    col.green = g<<8;
    col.blue = b<<8;
    if(col_depth == 24) c.l = (r<<16)|(g<<8)|b;
    else if(col_depth == 16) c.l = ((r&0xf8)<<8)|((g&0xfc)<<3)|((b>>3)&0x1f);
    else{
        c.l = col.pixel = (r<<16)|(g<<8)|b;
#ifdef SELF_COMPILE
        XAllocColor(d, colormap, &col);
        c.l = col.pixel;
#else
        //XQueryColor(disp, colormap, &col);
        c.l=ANDRES_RGB_Color(r,g,b);
#endif
        c.r = col.red>>8;
        c.g = col.green>>8;
        c.b = col.blue>>8;
    }
    return c;
}

Color Color_24From16(unsigned int col){
    Color c;
    c.r = (col>>8)&0xf8;
    c.g = (col>>3)&0xfc;
    c.b = (col&0x1f)<<3;
    c.l = c.r<<16 | c.g << 8 | c.b;
    return c;
}

unsigned int Color_16From24(unsigned int col){
    return Color_GetColor(col>>16, col>>8, col).l;
}

unsigned long Color_GetLongColor(unsigned char r, unsigned char g, unsigned char b){
//    switch(col_depth){
//    case 16:
//        return ((r&0xf8)<<8)|((g&0xfc)<<3)|((b>>3)&0x1f);
//    case 8:
//        return 0;
//    case 24:
//    default:
//        return (r<<16)|(g<<8)|b;
//    }
    return Color_GetColor(r, g, b).l;
}

void Color_SetDepth(int depth, Display *d){
    if(col_depth != 0) return;
    col_depth = depth;
    if(disp != NULL) XCloseDisplay(disp);
    disp = d;
    colormap = XDefaultColormap(disp, DefaultScreen(disp));
}

int Color_GetDepth(){
    return col_depth;
}
