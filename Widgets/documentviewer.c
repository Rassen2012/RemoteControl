#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
//#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

//#include "lib_tiff/lib_tiff.h"
#include "documentviewer.h"

#include <X11/Xutil.h>

#define DEF_INDENT_LEFT 20
#define DEF_INDENT_RIGHT 20
#define DEF_TEXTINTERVAL 10
#define DEF_PARINTERVAL (DEF_TEXTINTERVAL<<1)
#define DEF_FONT_SIZE 14
#define DEF_ABZ 40
#define GET_COLOR(r, g, b, d) Color_GetColor1(r, g, b, d)

#ifndef null
#define null NULL
#endif
#define zero 0

typedef struct Merge{
    int id;
    int count;
    int start_x;
    int start_y;
    XDMerge m;
}Merge;

char *ToLowerS(char *src, int len);

void New_Page(DocumentView *dv){
    dv->cur_x = dv->indent_left;
    dv->cur_y = dv->text_interval + dv->font_size;
    Widget_Resize(dv->widget, dv->widget->width, dv->s->widget->height);
    Widget_Move(dv->widget, dv->widget->x, 0);
    //Scroll_SetWidget(dv->s, dv->widget);
}

void Check_Height(DocumentView *dv){
    if(dv->cur_y > dv->widget->height){
        Widget_Resize(dv->widget, dv->widget->width, dv->cur_y + dv->s->widget->height);
        //Scroll_SetWidget(dv->s, dv->widget);
    }
}

void DocumentView_SetNewLine(DocumentView *dv){
    dv->cur_x = dv->indent_left;
    //int old_y = dv->cur_y;
    dv->cur_y += dv->text_interval + dv->font_size;
    //dv->cur_line = ++dv->lines;
//    dv->text_lines = (TextLine*)realloc(dv->text_lines, sizeof(TextLine)*(dv->lines+1));
//    dv->text_lines[dv->lines].chars = 0;
//    dv->text_lines[dv->lines].text = NULL;
    Check_Height(dv);
//    Graphics *g = dv->widget->g;
//    Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
//    Graphics_FillRectangle(g, 0, old_y, dv->widget->width, dv->cur_y - old_y);
//    Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
}

void DocumentView_SetAbzac(DocumentView *dv){
    dv->cur_x = DEF_ABZ;
    dv->cur_y += dv->par_interval + dv->font_size;
    //dv->cur_line = (dv->lines += 2);
//    dv->text_lines = (TextLine*)realloc(dv->text_lines, sizeof(TextLine)*(dv->lines+1));
//    dv->text_lines[dv->lines-1].chars = dv->text_lines[dv->lines].chars = 0;
//    dv->text_lines[dv->lines-1].text = dv->text_lines[dv->lines].text = NULL;
    Check_Height(dv);
}

void DocumentView_DrawText(DocumentView *dv, char *text, XDAlign align, char f, char *href){
    Graphics *g = dv->widget->g;
    int text_width = 0;
    char *cur_text = NULL;// = text;
    int t_len = strlen(text);
    int n = 0;
    char *line = NULL;
    //int line_len = 0;
    text_width = XTextWidth(g->fontInfo, text, t_len);
    if(align == XDAlign_Center){
        if(dv->cur_x == dv->indent_left) dv->cur_x = dv->doc_width/2 - text_width/2;
    }
    if(href) Graphics_SetColor(g, BLUE_COLOR(g->widget->display));
    int sy = 5;
    int syy = sy<<1;
    //if(f)   Graphics_SetColor(g, GET_COLOR(0, 0, 255, g->widget->display));
    if(text_width + (dv->cur_x - dv->indent_left) > dv->doc_width){
        int s;
        while(n < t_len){
            s = 0;
            do{
                cur_text = realloc(cur_text, s+2);
                cur_text[s++] = text[n++];
                cur_text[s] = 0;
            }while((text_width = XTextWidth(g->fontInfo, cur_text, s)) + (dv->cur_x - dv->indent_left) < dv->doc_width && n < t_len);
            if(n >= t_len){
                if(f){
                    Graphics_SetColor(g, BLUE_COLOR(g->widget->display));
                    Graphics_FillRectangle(g, dv->cur_x, dv->cur_y-dv->font_size-sy, text_width, dv->font_size+syy);
                    Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
                }
                Graphics_DrawText(g, dv->cur_x, dv->cur_y, cur_text);
                if(href) Graphics_DrawLine(g, dv->cur_x, dv->cur_y, dv->cur_x + text_width, dv->cur_y);
                if(f) Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
                dv->cur_x += text_width;
                if(dv->cur_x - dv->indent_left >= dv->doc_width) DocumentView_SetNewLine(dv);
            }
            else{
                line = strrchr(cur_text, ' ');
                if(line){
                    int it = (line - cur_text) + 1;
                    line = strndup(cur_text, it);
                    int lw = XTextWidth(g->fontInfo, line, strlen(line));
                    if(f){
                        //lw = XTextWidth(g->fontInfo, line, strlen(line));
                        Graphics_SetColor(g, BLUE_COLOR(g->widget->display));
                        Graphics_FillRectangle(g, dv->cur_x, dv->cur_y - dv->font_size-sy, lw, dv->font_size+syy);
                        Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
                    }
                    Graphics_DrawText(g, dv->cur_x, dv->cur_y, line);
                    if(href) Graphics_DrawLine(g, dv->cur_x, dv->cur_y, dv->cur_x + lw, dv->cur_y);
                    if(f) Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
                    DocumentView_SetNewLine(dv);
                    free(line);
                    line = &cur_text[it];
                    lw = XTextWidth(g->fontInfo, line, strlen(line));
                    if(f){
                        //lw = XTextWidth(g->fontInfo, line, strlen(line));
                        Graphics_SetColor(g, BLUE_COLOR(g->widget->display));
                        Graphics_FillRectangle(g, dv->cur_x, dv->cur_y - dv->font_size-sy, lw, dv->font_size+syy);
                        Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
                    }
                    Graphics_DrawText(g, dv->cur_x, dv->cur_y, line);
                    if(href) Graphics_DrawLine(g, dv->cur_x, dv->cur_y, dv->cur_x + lw, dv->cur_y);
                    if(f) Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
                    dv->cur_x += XTextWidth(g->fontInfo, line, s-it);
                }
                else{
                    int lw = XTextWidth(g->fontInfo, cur_text, strlen(cur_text));
                    if(f){
                        //int lw = XTextWidth(g->fontInfo, cur_text, strlen(cur_text));
                        Graphics_SetColor(g, BLUE_COLOR(g->widget->display));
                        Graphics_FillRectangle(g, dv->cur_x, dv->cur_y - dv->font_size-sy, lw, dv->font_size+syy);
                        Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
                    }
                    Graphics_DrawText(g, dv->cur_x, dv->cur_y, cur_text);
                    if(href) Graphics_DrawLine(g, dv->cur_x, dv->cur_y, dv->cur_x + lw, dv->cur_y);
                    if(f)   Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
                    DocumentView_SetNewLine(dv);
                }
            }
            if(cur_text){
                free(cur_text);
                cur_text = NULL;
            }
        }
    }
    else{
        if(f){
            Graphics_SetColor(g, BLUE_COLOR(g->widget->display));
            Graphics_FillRectangle(g, dv->cur_x, dv->cur_y - dv->font_size-sy, text_width, dv->font_size+syy);
            Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
        }
        Graphics_DrawText(g, dv->cur_x, dv->cur_y, text);
        if(href) Graphics_DrawLine(g, dv->cur_x, dv->cur_y, dv->cur_x + text_width, dv->cur_y);
        if(f) Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
        dv->cur_x += text_width;
        if(dv->cur_x - dv->indent_left >= dv->doc_width) DocumentView_SetNewLine(dv);
    }
    Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
}

void DocumentView_DrawParagraph(DocumentView *dv, XDParagraph *par){
    int par_count = 0;
    if(strcmp(par->text, "\n") == 0){
        DocumentView_SetNewLine(dv);
        return;
    }
    char *text = par->text;
    if(dv->lpar != NULL && dv->lpar == par){
        dv->ly = dv->cur_y;
    }
    int old_fp = dv->fp;
    if(par->isList){
//        DocumentView_DrawText(dv, par->listTxt, par->properties.align);
//        DocumentView_DrawText(dv, " ", par->properties.align);
        int t_len = strlen(par->text);
        int l_len = strlen(par->listTxt);
        text = (char*)malloc(t_len + l_len + 2);
        memcpy(text, par->listTxt, l_len);
        memcpy(&text[l_len], " ", 1);
        memcpy(&text[l_len+1], par->text, t_len);
        text[l_len+t_len + 1] = 0;
        if(dv->fp >= 0) dv->fp += l_len +1;
        //text = calloc(t_len + l_len + 2, 1);
        //strncpy(text, par->listTxt, l_len);
        //strncpy(&text[l_len], " ", 1);
        //strncpy(&text[l_len+1], par->text, t_len);
    }
    char **pars = XDocument_Split(par->text, strlen(text), "\n", &par_count);
    char f = 0;
    char *href = par->href;
    if(href){
        dv->links = realloc(dv->links, sizeof(LinkRect) * (dv->links_count+1));
        dv->links[dv->links_count].rect.x = dv->cur_x;
        dv->links[dv->links_count].rect.y = dv->cur_y;
        dv->links[dv->links_count].href = href;
        dv->links[dv->links_count].hov = 0;
    }
    if(par_count == 0){
        if(dv->fpar && dv->fpar == par && dv->ftext){
            char *tmptext = ToLowerS(text, strlen(text));
            char *ttt = dv->ftext;//strdup(dv->ftext);//strstr(&tmptext[dv->fp], dv->ftext);
            if(ttt){
                //f = 1;
                int ssize = dv->fp;//ttt - tmptext;
                free(tmptext);
                //char *ft = (char*)calloc(ssize, 1);
                //strncpy(ft, text, ssize);
                //char *ft = (char*)malloc(ssize);
                //strncpy(ft, text, ssize);
                char *ft = malloc(ssize+1);
                memcpy(ft, text, ssize);
                ft[ssize] = 0;
                DocumentView_DrawText(dv, ft, par->align, f, href);
                free(ft);
                f = 1;
                //free(ft);
                //ft = (char*)calloc(strlen(dv->ftext)+1, 1);
                //strncpy(ft, &text[ssize], strlen(dv->ftext));
                ft = dv->ftext;
                DocumentView_DrawText(dv, ft, par->align, f, href);
                //free(ft);
                dv->fy = dv->cur_y;
//                if(dv->cur_y > dv->s->widget->height){
//                    int mov_per = (100 * dv->cur_y)/dv->widget->height;
//                    Slider_MovePer(dv->s->slVer, mov_per);
//                    //Slider_Move(dv->s->slVer, mov_per);
//                }
                int tmpsize = strlen(text) - ssize - strlen(dv->ftext);
                ft = (char*)calloc(tmpsize + 1, 1);
                strncpy(ft, &text[ssize + strlen(dv->ftext)], tmpsize);
                f = 0;
                ft[tmpsize] = 0;
                DocumentView_DrawText(dv, ft, par->align, f, href);
                free(ft);
            }
            else{
                DocumentView_DrawText(dv, text, par->align, f, href);
            }
        }
        else DocumentView_DrawText(dv, text, par->align, f, href);
    }
    else{
        int i;
        for(i = 0; i < par_count; i++){
            if(dv->fpar && dv->fpar == par && dv->ftext){
                char *ttt = strstr(text, dv->ftext);
                if(ttt){
                    //f = 1;
                    int ssize = ttt - text;
                    char *ft = (char*)calloc(ssize, 1);
                    strncpy(ft, text, ssize);
                    DocumentView_DrawText(dv, ft, par->align, f, href);
                    f = 1;
                    free(ft);
                    ft = (char*)calloc(strlen(dv->ftext), 1);
                    strncpy(ft, &text[ssize], strlen(dv->ftext));
                    DocumentView_DrawText(dv, ft, par->align, f, href);
                    free(ft);
                    int tmpsize = strlen(text) - ssize - strlen(dv->ftext);
                    ft = (char*)calloc(tmpsize + 1, 1);
                    strncpy(ft, &text[ssize + strlen(dv->ftext)], tmpsize);
                    f = 0;
                    DocumentView_DrawText(dv, ft, par->align, f, href);
                    free(ft);
                }
                else{
                    DocumentView_DrawText(dv, text, par->align, f, href);
                }
            }
            else DocumentView_DrawText(dv, pars[i], par->align, f, href);
            DocumentView_SetAbzac(dv);
            free(pars[i]);
        }
        free(pars);
    }
    dv->fp = old_fp;
    if(par->isList) free(text);
    if(href){
        dv->links[dv->links_count].rect.width = dv->cur_x;
        dv->links[dv->links_count++].rect.height = dv->cur_y;
    }
}

void DocumentView_PutImage(DocumentView *dv, XDImage *im){
    XImage *image = NULL;
    Graphics *g = dv->widget->g;
//    if(im->image){
//        XImage *image = im->image;
//        XPutImage(g->widget->display, g->context, g->gc, image, 0, 0, dv->cur_x, dv->cur_y, image->width, image->height);
//    }
//    int res = lib_tiff_open_tiff_file(im->fileName, g->widget->display, DefaultScreen(g->widget->display), &image);
//    if(res == 1){
//        XPutImage(g->widget->display, g->context, g->gc, image, 0, 0, dv->cur_x, dv->cur_y, image->width, image->height);
//        XDestroyImage(image);
//    }
}

void DocumentView_DrawImage(DocumentView *dv, XDImage *im){
    XImage *image = NULL;
    Graphics *g = dv->widget->g;
    int res = 0;//lib_tiff_open_tiff_file(im->fileName, g->widget->display, DefaultScreen(g->widget->display), &image);
    if(res == 1){
    //if(im->image){
        /*if(im->align == XDAlign_Left){
            XPutImage(g->widget->display, g->context, g->gc, image, 0, 0, dv->cur_x, dv->cur_y, image->width, image->height);
            dv->cur_x += image->width;
            if(dv->cur_x - dv->indent_left >= dv->doc_width) DocumentView_SetNewLine(dv);
        }*/
        //XImage *image = im->image;
        //if(dv->cur_y + image->height > dv->widget->height){

           // Widget_Resize(dv->widget, dv->widget->width, dv->widget->height + image->height + dv->text_interval + dv->font_size);
            //Scroll_SetWidget(dv->s, dv->widget);
        //}
        dv->cur_y += image->height;
        Check_Height(dv);
        dv->cur_y -= image->height;
        if(im->align == XDAlign_Center){
            DocumentView_SetNewLine(dv);
            dv->cur_x = dv->doc_width/2 - image->width/2 + dv->indent_left;
            XPutImage(g->widget->display, g->context, g->gc, image, 0, 0, dv->cur_x,dv->cur_y-dv->font_size, image->width, image->height);
            dv->cur_y += image->height;
            //Check_Height(dv);
            DocumentView_SetNewLine(dv);
        }
        else{
            XPutImage(g->widget->display, g->context, g->gc, image, 0, 0, dv->cur_x, dv->cur_y-dv->font_size, image->width, image->height);
            dv->cur_x += image->width;
            if(dv->cur_x - dv->indent_left >= dv->doc_width){ dv->cur_y += image->height; DocumentView_SetNewLine(dv); }
        }
        XDestroyImage(image);
    }
}

void DocumentView_InitTable(DocumentView *dv, XDTable *table){
    int i, j;
    Graphics *g = dv->widget->g;
    table->height = 0;
    for(i = 0; i < table->row_count; i++){
        XDRow *row = table->rows[i];
        int height = 0;
        row->height = height;
        for(j = 0; j < row->cells_count; j++){
            XDCell *cell = row->cells[j];
            if(cell->height < dv->font_size) cell->height = dv->font_size + dv->text_interval*2;
            height = cell->height;
            if(cell->text != NULL && cell->tag->tags_count <= 0){
                int text_width = XTextWidth(g->fontInfo, cell->text, strlen(cell->text));
                if(text_width > cell->width){
                    height *= (text_width/cell->width + 1);
                }
                //if(height > row->height) row->height = height;
            }
            else if(cell->tag->tags_count > 0){
                //XDImage *image = (XDImage*)cell->tag->tags[0]->Object;
                //if(image->height > height) height = image->height + dv->text_interval*2;
                //if(height > row->height) row->height = height;
                int n;
                for(n = 0; n < cell->tag->tags_count; n++){
                    XDTag *tag = cell->tag->tags[n];
                    if(tag->type == TagType_Paragraph && cell->tag->tags_count > 1){
//                        height += dv->font_size + dv->text_interval;
//                        cell->height = height;
                    }
                }
            }
            if(height > row->height) row->height = height;
        }
        if(row->height < dv->font_size) row->height = dv->font_size + (dv->par_interval<<1);
        table->height += row->height;
    }
}

void DocumentView_DrawTable(DocumentView *dv, XDTable *table){
    int i, j;//, hm;
    //Merge *m = NULL;
    //DocumentView_SetNewLine(dv);
    DocumentView_InitTable(dv, table);
    Graphics *g = dv->widget->g;
    Color backcolor = BLACK_COLOR(g->widget->display);
    Graphics_SetColor(g, backcolor);
    if(table->height + dv->cur_y > dv->widget->height){
        //Widget_Resize(dv->widget, dv->widget->width, dv->widget->height + table->height + dv->text_interval + dv->font_size);
        //Scroll_SetWidget(dv->s, dv->widget);
        dv->cur_y += table->height;
        Check_Height(dv);
        dv->cur_y -= table->height;
    }
    for(i = 0; i < table->row_count; i++){
        XDRow *row = table->rows[i];
        int height = 0;
        Rectangle *rect = NULL;
        int rect_count = 0;
        for(j = 0; j < row->cells_count; j++){
            XDCell *cell = row->cells[j];
            //if(cell->height < height || height == 0) height = cell->height;
            height = row->height;//cell->height;
            //Graphics_DrawRectangle(g, dv->cur_x, dv->cur_y, cell->width, height);
            int old_x = dv->cur_x;
            int old_y = dv->cur_y;
            int old_w = dv->doc_width;
            dv->doc_width = cell->width;
            dv->cur_y += dv->font_size + 2;//(height/2 + dv->font_size/2);
            dv->cur_x += 2;
            if(cell->tag != NULL && cell->tag->tags_count > 0){
                int n;
                dv->cur_y = old_y + height/2;
                for(n = 0; n < cell->tag->tags_count; n++){
//                    XDImage *image = (XDImage*)cell->tag->tags[n]->Object;
//                    dv->cur_y -= (image->height/2);
//                    DocumentView_PutImage(dv, image);
                    if(cell->tag->tags[n]->type == TagType_Image){
                        XDImage *image = (XDImage*)cell->tag->tags[n]->Object;
                        dv->cur_y -= (image->height/2);
                        DocumentView_PutImage(dv, image);
                        dv->cur_y += (image->height);
                    }
                    else{
                        XDParagraph *p = (XDParagraph*)cell->tag->tags[n]->Object;
                        int left_indent = dv->indent_left;
                        dv->indent_left = dv->cur_x;
                        int text_interval = dv->text_interval;
                        dv->text_interval = 2;
                        DocumentView_DrawText(dv, p->text, XDAlign_Left, 0, NULL);
                        dv->indent_left = left_indent;
                        dv->text_interval = text_interval;
                    }
                }
            }
//            else{ //DocumentView_DrawText(dv, cell->text, XDAlign_Center);
//                int left_indent = dv->indent_left;
//                dv->indent_left = dv->cur_x;
//                DocumentView_DrawText(dv, cell->text, XDAlign_Left, 0, NULL);
//                dv->indent_left = left_indent;
//            }
//            if(dv->cur_y == old_y && dv->cur_x - old_x > cell->width) cell->width = (dv->cur_x - old_x) + 4;
            if(dv->cur_y - old_y > cell->height) cell->height = (dv->cur_y - old_y) + 4;
            //Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
            //Graphics_DrawRectangle(g, old_x, old_y, cell->width, cell->height+5);
            rect = (Rectangle*)realloc(rect, sizeof(Rectangle)*(rect_count+1));
            rect[rect_count].x = old_x;
            rect[rect_count].y = old_y;
            rect[rect_count].height = cell->height+5;
            rect[rect_count++].width = cell->width;
            dv->cur_x = old_x + cell->width;
            dv->cur_y = old_y;
            dv->doc_width = old_w;
        }
        int ns, nj;
        Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
        for(ns = 0; ns < rect_count; ns++){
            for(nj = 0; nj < rect_count; nj++){
                if(nj + 1 < rect_count && rect[nj].height != rect[nj+1].height) {
                    if(rect[nj].height < rect[nj+1].height) rect[nj].height = rect[nj+1].height;
                    else rect[nj+1].height = rect[nj].height;
                }
            }
        }
        for(ns = 0; ns < rect_count; ns++){
            Graphics_DrawRectangle(g, rect[ns].x, rect[ns].y, rect[ns].width, rect[ns].height);
            height = rect[ns].height;
        }
        dv->cur_x = dv->indent_left;
        dv->cur_y += height;//+5;
    }
    //Graphics_SetFontSize(g, 14);
    DocumentView_SetNewLine(dv);
}

void DocumentView_DrawPage(DocumentView *dv){
    int i;
    unsigned long tt, tpar_all, tim_all, ttab_all;
    tpar_all = tim_all = ttab_all = 0;
    for(i = 0; i < dv->page->tags_count; i++){
        XDTag *tag = dv->page->tags[i];
        switch(tag->type){
        case TagType_Paragraph:
            tt = time_get();
            DocumentView_DrawParagraph(dv, (XDParagraph*)tag->Object);
            tpar_all += time_get() - tt;
            break;
        case TagType_Image:
            tt = time_get();
            DocumentView_DrawImage(dv, (XDImage*)tag->Object);
            tim_all += time_get() - tt;
            break;
        case TagType_Table:
            tt = time_get();
            DocumentView_DrawTable(dv, (XDTable*)tag->Object);
            ttab_all += time_get() - tt;
            break;
        default:
            break;
        }
    }
    printf("Time to draw paragraphs - %lu\n", tpar_all);
    printf("Time to draw images - %lu\n", tim_all);
    printf("Time to draw tables - %lu\n", ttab_all);
    tt = time_get();
    Scroll_SetWidget(dv->s, dv->widget);
    printf("Time to scroll - %lu\n", time_get() - tt);
}

DocumentView *DocumentView_newDocumentView(S_Widget *parent, int x, int y, int width, int height){
    DocumentView *dv = (DocumentView*)malloc(sizeof(DocumentView));
    //height = 78600;//7560;
    dv->s = Scroll_newScroll(parent, x, y, parent->width, parent->height, VERTICAL_SCROLL | HORIZONTAL_SCROLL);
    dv->widget = Widget_newWidget(x, y, width, height, dv->s->widget);
    dv->widget->type = WIDGET_TYPE_DOCVIEW;
    dv->widget->object = (void*)dv;

    dv->page = null;
    dv->doc = null;
    dv->font_size = DEF_FONT_SIZE;
    dv->indent_left = dv->indent_right = DEF_INDENT_LEFT;
    dv->cur_line = zero;
    dv->lines = zero;
    dv->cur_x = DEF_INDENT_LEFT;
    dv->cur_y = DEF_TEXTINTERVAL + DEF_FONT_SIZE;
    dv->par_interval = DEF_PARINTERVAL;
    dv->text_interval = DEF_TEXTINTERVAL;
    dv->doc_width = dv->widget->width - (dv->indent_left+dv->indent_right);
    //dv->text_lines = NULL;
    dv->drawing = 0;
    dv->fpar = NULL;
    dv->ftext = NULL;
    dv->fj = 0;
    dv->fi = 0;
    dv->fp = -1;
    dv->find = 0;
    dv->lv = NULL;
    dv->focus = 0;
    dv->SelectItem = NULL;
    dv->links = NULL;
    dv->links_count = 0;
    dv->ly = 0;
    dv->lpar = NULL;

    dv->defCur = XCreateFontCursor(dv->widget->display, XC_left_ptr);
    dv->handCur = XCreateFontCursor(dv->widget->display, XC_hand1);
    dv->hand = 0;

    XSelectInput(dv->widget->display, dv->widget->window, ExposureMask | KeyPressMask | ButtonPressMask | PointerMotionMask);

    return dv;
}

void DocumentView_Dispose(DocumentView *dv){
    if(!dv->widget->disposed) Widget_Dispose(dv->widget);
    if(dv->doc != null) XDocument_Dispose(dv->doc);
    if(dv->links) free(dv->links);
    free(dv);
}

void DocumentView_SetDocument(DocumentView *dv, XDocument *doc){
    dv->doc = doc;
}

void DocumentView_LoadDocument(DocumentView *dv, char *filename){
    dv->doc = XDocument_newDocument(dv->widget->display);
    unsigned long time = time_get();
    //XDocument_LoadFile(dv->doc, filename);
    XDocument_LoadBinFile(dv->doc, filename);
    time = time_get() - time;
    printf("Time to load document - %lu\n", time);
    time = time_get();
    if(dv->doc->pages_count > 0) DocumentView_SetPage(dv, dv->doc->pages[0]);
    time = time_get() - time;
    printf("Time to set page - %lu\n", time);
}

void DocumentView_SetPage(DocumentView *dv, XDPage *p){
    dv->page = p;
    dv->drawing = 0;
    //dv->fpar = fpar;
    //dv->ftext = ftext;
    dv->fy = dv->ly = 0;
    if(dv->links) free(dv->links);
    if(!dv->find && dv->fpar){
        dv->fpar = NULL;
        dv->ftext = NULL;
        dv->fi = dv->fj = 0;
        dv->fp = -1;
    }
    dv->links = NULL;
    dv->links_count = 0;
    New_Page(dv);
    DocumentView_Paint(dv, NULL);
    //DocumentView_Paint(dv, NULL);
    dv->drawing = 1;
    if(dv->fy > dv->s->widget->height){
        int mov_per = (100 * dv->fy)/dv->widget->height;
        Slider_MovePer(dv->s->slVer, mov_per);
    }
    else if(dv->ly > dv->s->widget->height){
        int mov_per = (100 * dv->ly)/dv->widget->height;
        Slider_MovePer(dv->s->slVer, mov_per);
    }
    if(dv->find) dv->find = 0;
    //DocumentView_DrawPage(dv);
}

void DocumentView_SetPageFromId(DocumentView *dv, char *id){
    int i;
    for(i = 0; i < dv->doc->pages_count; i++){
        if(strcmp(dv->doc->pages[i]->link, id) == 0){
            DocumentView_SetPage(dv, dv->doc->pages[i]);
            break;
        }
    }
}

void DocumentView_SetFontSize(DocumentView *dv, short fsize){
    Graphics_SetFontSize(dv->widget->g, fsize);
    dv->font_size = fsize;
    DocumentView_DrawPage(dv);
}

void DocumentView_Paint(DocumentView *dv, PaintEventArgs *ev){
    Graphics *g = dv->widget->g;
    if(dv->drawing){
        //Graphics_BeginPaint(g, ev);
        Graphics_SetColor(g, WHITE_COLOR(g->widget->display));
        XFillRectangle(g->widget->display, g->widget->window, g->gc, 0, 0, g->widget->width, g->widget->height);
        XCopyArea(g->widget->display, g->context, g->widget->window, g->gc, 0, 0, g->widget->width, g->widget->height, 0, 0);
        //Graphics_EndPaint(g);
        return;
    }
    Graphics_BeginPaint(g, NULL);
    Color backcolor = WHITE_COLOR(g->widget->display);
    Graphics_SetColor(g, backcolor);
    Graphics_FillRectangle(g, 0, 0, g->widget->width, g->widget->height);
    Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
    dv->cur_x = dv->indent_left;
    dv->cur_y = DEF_PARINTERVAL;
    unsigned long tt = time_get();
    if(dv->page) DocumentView_DrawPage(dv);//DocumentView_DrawPage(dv);
    tt = time_get() - tt;
    printf("Time to draw page - %lu\n", tt);
    Graphics_EndPaint(g);
}

void DocumentView_SetPageEvent(void *sender, void *link){
    unsigned long tt = time_get();
    DocumentView_SetPageFromId((DocumentView*)sender, (char*)link);
    printf("Set page time - %lu\n", time_get() - tt);
}

unsigned char ToLower(unsigned char ch){
    static unsigned char lower_case[256];
    static char init = 0;
    if(!init){
        int i;
        for(i = 0; i < 256; i++) lower_case[i] = i;
        unsigned char *rus_upper = "áâ÷çäå³öúéêëìíîïðòóôõæèãþûýÿùøüàñ";
        unsigned char *rus_lower = "ÁÂ×ËÄÅ£ÖÚÉÊËÌÍÎÏÐÒÓÔÕÆÈÃÞÛÝßÙØÜÀÑ";
        unsigned char *eng_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        unsigned char *eng_lower = "abcdefghijklmnopqrstuvwxyz";
        for(i = 0; i < (int)strlen((char*)rus_upper); i++) lower_case[rus_upper[i]] = rus_lower[i];
        for(i = 0; i < (int)strlen((char*)eng_upper); i++) lower_case[eng_upper[i]] = eng_lower[i];
        init = 1;
    }
    return lower_case[ch];
}

char *ToLowerS(char *src, int len){
    int i;
    char *res = (char*)malloc(len+1);
    for(i = 0; i < len; i++){
        res[i] = ToLower((unsigned char)src[i]);
    }
    res[len] = 0;
    return res;
}

void DocumentView_Find(void *object, char *ttext){
    if(ttext == NULL) return;
    DocumentView *dv = (DocumentView*)object;
    int i, j;//, n;
    char *text = ToLowerS(ttext, strlen(ttext));

    if(dv->fpar != NULL && dv->ftext != NULL && strcmp(dv->ftext, text) != 0){
        dv->fi = 0;
        dv->fj = 0;
        dv->fp = -1;
        dv->fpar = NULL;
        dv->ftext = NULL;
    }
    for(i = 0; i < dv->doc->pages_count; i++){
        XDPage *page = dv->doc->pages[i];
        if(dv->fi > i) continue;
        if(dv->fi < i) {
            dv->fj = 0;
            dv->fp = -1;
        }
        for(j = 0; j < page->tags_count; j++){
            if(dv->fj > j) continue;
            if(dv->fj < j){
                dv->fp = -1;
            }
            if(page->tags[j]->type == TagType_Paragraph){
                XDParagraph *par = (XDParagraph*)page->tags[j]->Object;
                char *tmptext = ToLowerS(par->text, strlen(par->text));
                //if(strstr(par->text, text) != NULL){
                char *tval = NULL;
//                if(dv->fpar != par){
//                    dv->fp = -1;
//                }
                if(dv->fp >= 0) tval = strstr(&tmptext[dv->fp+1], text);
                else tval = strstr(tmptext, text);
                if(tval){
                    int tl = (tval - tmptext);
                    //if((dv->fpar == par && dv->fi == i && (dv->fp == tl || dv->fp == -1)) || (dv->fi >= i && dv->fj >= j && dv->fp >= tl)) continue;
                    if(dv->fpar == par && dv->fi == i && dv->fp == tl) continue;
                    if(dv->fi > i || dv->fj > j || dv->fp > tl) continue;
                    dv->fi = i;
                    dv->fj = j;
                    dv->fpar = par;
                    dv->ftext = text;
                    dv->fp = tl;
                    dv->find = 1;
                    if(dv->SelectItem) dv->SelectItem(dv->lv, page->link);
                    return;
                }
//                if(strstr(tmptext, text) != NULL){
//                    if((dv->fpar == par && dv->fi == i) || (dv->fi >= i && dv->fj > j)) continue;
//                    //if(dv->fpar == par || dv->fi > i || (dv->fi >= i && dv->fj > j)) continue;
////                    int count = 0;
////                    char **ttext = XDocument_Split(par->text, strlen(par->text), " ", &count);
////                    for(n = 0; n < count; n++){
////                        if(strcmp(ttext[n], text) == 0){
////                            DocumentView_SetPage(dv, page);
////                            return;
////                        }
////                    }
//                    dv->fi = i;
//                    dv->fj = j;
//                    dv->fpar = par;
//                    dv->ftext = text;
//                    //DocumentView_SetPage(dv, page, par, text);
//                    if(dv->SelectItem) dv->SelectItem(dv->lv, page->link);
//                    return;
//                }
            }
        }
    }
    dv->fi = 0;
    dv->fj = 0;
    dv->fp = -1;
}

void DocumentView_Tab(DocumentView *dv){
    dv->focus = 1;
}

void DocumentView_Untab(DocumentView *dv){
    dv->focus = 0;
}

void DocumentView_KeyPress(DocumentView *dv, KeyEventArgs *ev){
    if(dv->focus && dv->s->slVer->widget->visible){
        if(ev->ksym == XK_Up){
            Slider_Move(dv->s->slVer, -10);
        }
        else if(ev->ksym == XK_Down){
            Slider_Move(dv->s->slVer, 10);
        }
        else if(ev->ksym == XK_Page_Up){
            //char *text = " t";
            //char *t = strstr(text, " ");
            //int l = t - text;
            //return;
//            int per = ((0 - (dv->widget->y + dv->s->widget->height))*100)/dv->widget->height;
//            int val = (per*(dv->s->slVer->widget->height - 30 - dv->s->slVer->size))/100;
//            int dval = dv->s->slVer->slid_y - val;
//            Slider_Move(dv->s->slVer, dval*(-1));
            //dval = -1;
            int per = ((0 - (dv->widget->y))*100)/dv->widget->height;
            int per2 = ((0 - (dv->widget->y + dv->s->widget->height))*100)/dv->widget->height;

            per = per2 - per;
            if(per != 0) Slider_MovePer(dv->s->slVer, per);
            else Slider_MovePer(dv->s->slVer, -100);
        }
        else if(ev->ksym == XK_Page_Down){
            float per = ((dv->widget->y - dv->s->widget->height)*100.0f)/(float)dv->widget->height;
            float per2 = ((float)dv->widget->y*100.0f)/(float)dv->widget->height;
            per = per2 - per;
            Slider_MovePer(dv->s->slVer, per);
            //int val = (per*(dv->s->slVer->widget->height - 30 - dv->s->slVer->size))/100;
            //int dval = dv->s->slVer->slid_y - val;
            //Slider_Move(dv->s->slVer, dval);
            //Slider_Move(dv->s->slVer, dv->s->widget->height);
            //Slider_MovePer(dv->s->slVer, per);
        }
    }
}

void DocumentView_MousePress(DocumentView *dv, MouseEventArgs *ev){
    if(ev->button == MOUSE_BUTTON_LEFT){
        Widget_Tab(dv->widget);
        if(dv->hand){
            int i, j, n;
            for(i = 0; i < dv->links_count; i++){
                if(dv->links[i].hov){
                    for(j = 0; j < dv->doc->pages_count; j++){
                        XDPage *page = dv->doc->pages[j];
                        for(n = 0; n < page->tags_count; n++){
                            if(page->tags[n]->type == TagType_Paragraph){
                                XDParagraph *par = (XDParagraph*)page->tags[n]->Object;
                                if(par->link != NULL && strcmp(par->link, dv->links[i].href) == 0){
                                    dv->lpar = par;
                                    if(dv->SelectItem) dv->SelectItem(dv->lv, page->link);
                                    return;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

void DocumentView_MouseMove(DocumentView *dv, MouseEventArgs *ev){
    char hand = 0;
    if(dv->links_count > 0){
        int i;
        for(i = 0; i < dv->links_count; i++){
            dv->links[i].hov = 0;
            if(ev->y >= dv->links[i].rect.y - dv->font_size && ev->y <= dv->links[i].rect.height){
                if(dv->links[i].rect.y == dv->links[i].rect.height){
                    if(ev->x >= dv->links[i].rect.x && ev->x <= dv->links[i].rect.width){
                        hand = 1;
                        dv->links[i].hov = 1;
                    }
                }
                else{
                    if((ev->y <= dv->links[i].rect.y && ev->x >= dv->links[i].rect.x) ||
                            (ev->y > dv->links[i].rect.y && ev->y < dv->links[i].rect.height - dv->font_size) ||
                            (ev->y > dv->links[i].rect.y && ev->x <= dv->links[i].rect.width)){
                        hand = 1;
                        dv->links[i].hov = 1;
                    }
                }
            }
        }
    }
    if(hand && !dv->hand){
        XUndefineCursor(dv->widget->display, dv->widget->window);
        XDefineCursor(dv->widget->display, dv->widget->window, dv->handCur);
        dv->hand = 1;
    }
    else if(!hand && dv->hand){
        XUndefineCursor(dv->widget->display, dv->widget->window);
        XDefineCursor(dv->widget->display, dv->widget->window, dv->defCur);
        dv->hand = 0;
    }
}
