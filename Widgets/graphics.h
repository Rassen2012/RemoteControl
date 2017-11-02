#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <X11/Xlib.h>
#include "events.h"
#include "color.h"

struct S_Widget;

typedef struct Rectangle{
    int x, y;
    int width, height;
}Rectangle;

typedef struct Graphics{
    struct S_Widget *widget;
    GC gc;
    Pixmap context;
    XFontStruct *fontInfo;
    PaintEventArgs *ev;
    Color cur_color;
}Graphics;

Graphics *Graphics_newGraphics(struct S_Widget *widget);
void Graphics_Dispose(Graphics *g);
void Graphics_SetColor(Graphics *g, Color c);
void Graphics_SetFont(Graphics *g, char *font);
void Graphics_SetFontSize(Graphics *g, int size);
void Graphics_Resize(Graphics *g, ResizeEventArgs *ev);

void Graphics_BeginPaint(Graphics *g, PaintEventArgs *ev);
void Graphics_EndPaint(Graphics *g);
void Graphics_SetLineWidth(Graphics *g, int width);
void Graphics_DrawLine(Graphics *g, int x1, int y1, int x2, int y2);
void Graphics_DrawPoint(Graphics *g, int x, int y);
void Graphics_DrawRectangle(Graphics *g, int x, int y, int width, int height);
void Graphics_DrawEllips(Graphics *g, int x_center, int y_center, int width, int height);
void Graphics_DrawArc(Graphics *g, int x, int y, int width, int height, int angle1, int angle2);
void Graphics_DrawText(Graphics *g, int x, int y, const char *text);
void Graphics_DrawImage(Graphics *g, Pixmap image, int x, int y, int width, int height, int xd, int yd);
void Graphics_FillRectangle(Graphics *g, int x, int y, int width, int height);
void Graphics_FillEllips(Graphics *g, int x_center, int y_center, int width, int height);
Pixmap Graphics_LoadImage(Graphics *g, char *path);
void Graphics_FillPolygon(Graphics *g, XPoint *points, int npoints);
void Graphics_FillTriangle(Graphics *g, XPoint *points);
void Graphics_DrawSText(Graphics *g, char *text);

char *Graphics_RedepthImage(char *src, int src_size, int src_depth, int dest_depth, unsigned int **pal, unsigned char *big_pal);
void Graphics_RleDecompress(char **old, char *src, int src_size, char *dst, int *dst_size, int n);

#endif // GRAPHICS_H

