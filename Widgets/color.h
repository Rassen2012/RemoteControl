#ifndef COLOR_H
#define COLOR_H

#define WHITE_COLOR(d) Color_GetColor1(255, 255, 255, d)
#define BLACK_COLOR(d) Color_GetColor1(0, 0, 0, d)
#define RED_COLOR(d) Color_GetColor1(255, 0, 0, d)
#define GREEN_COLOR(d) Color_GetColor1(0, 255, 0, d)
#define BLUE_COLOR(d) Color_GetColor1(0, 0, 255, d)

#ifndef OC2K1x
typedef struct _XDisplay Display;
#endif

typedef struct Color{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned long l;
}Color;

Color Color_GetColor(unsigned char r, unsigned char g, unsigned char b);
Color Color_GetColor1(unsigned char r, unsigned char g, unsigned char b, Display *d);
unsigned long Color_GetLongColor(unsigned char r, unsigned char g, unsigned char b);
void Color_SetDepth(int depth, Display *d);
int Color_GetDepth();
Color Color_24From16(unsigned int col);
unsigned int Color_16From24(unsigned int col);

#endif // COLOR_H

