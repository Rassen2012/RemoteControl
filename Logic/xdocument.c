#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
//#include <X11/Xutil.h>

//#include "../Widgets/lib_tiff/lib_tiff.h"
#include "system.h"
#include "xdocument.h"

typedef enum TagsId{
    Content = 0x01,
    Paragraph = 0x02,
    Table = 0x03,
    TableRow = 0x04,
    TableCell = 0x05,
    Image = 0x06,
    Property = 0x07,
    List = 0x08
}TagsId;

void XDocument_DisposeContent(XDContent *content);
//void XDocument_DisposeTag(XDTag *tag);
void XDocument_SplitTags(XDocument *doc);
void XDocument_AddTag(XDocument *doc, XDTag *tag);
void XDocument_SplitProperty(XDTag *tag);
void XDocument_SetParagraph(XDTag *tag);
void XDocument_SetTable(XDTag *tag);
void XDocument_SetImage(XDTag *tag);
void XDocument_SplitPages(XDocument *doc);

char *XDocument_Trim(char *doc, long *len);

Display *disp;

XDocument *XDocument_newDocument(Display *d){
    XDocument *doc = (XDocument*)malloc(sizeof(XDocument));
    if(doc == NULL) return NULL;
    doc->content = NULL;
    doc->doc = NULL;
    doc->fileName = NULL;
    doc->tags = NULL;
    doc->tags_count = doc->doc_size = 0;
    doc->d = d;
    disp = d;
    return doc;
}

void XDocument_Dispose(XDocument *doc){
    int i, j;
    //if(doc->tags_count > 0 && doc->tags != NULL) free(doc->tags);
    if(doc->fileName != NULL) free(doc->fileName);
    //if(doc->doc != NULL) free(doc->doc);
    if(doc->content != NULL) XDocument_DisposeContent(doc->content);

    if(doc->pages != NULL){
        for(i = 0; i < doc->pages_count; i++){
            if(doc->pages[i]->tags != NULL) {
                for(j = 0; j < doc->pages[i]->tags_count; j++) XDocument_DisposeTag(doc->pages[i]->tags[j]);
                free(doc->pages[i]->tags);
            }
        }
        free(doc->pages);
    }
//    if(doc->tags != NULL){
//        for(i = 0; i < doc->tags_count; i++){
//            XDocument_DisposeTag(doc->tags[i]);
//        }
//    }
    if(doc->tags_count > 0 && doc->tags != NULL) free(doc->tags);
    if(doc->doc) free(doc->doc);
    free(doc);
}

void XDocument_DisposeContent(XDContent *content){
    int i;
    for(i = 0; i < content->parts_count; i++){
        if(content->chapters[i].link != NULL) free(content->chapters[i].link);
        if(content->chapters[i].title != NULL) free(content->chapters[i].title);
    }
    free(content->chapters);
    //if(text[off-1] == '|' && text[off+1] == '|') { off++; s++; continue; }
    free(content);
}

void XDocument_DisposeTable(XDTable *table){
    int i, j;
    for(i = 0; i < table->row_count; i++){
        for(j = 0; j < table->rows[i]->cells_count; j++){
            free(table->rows[i]->cells[j]);
        }
        free(table->rows[i]);
    }
    free(table);
}

void XDocument_DisposeImage(XDImage *image){
    //XDestroyImage(image->image);
    free(image);
}

void XDocument_DisposeParagraph(XDParagraph *par){
    if(par->href) free(par->href);
    if(par->link) free(par->link);
    if(par->listTxt) free(par->listTxt);
    free(par);
}

void XDocument_DisposeTag(XDTag *tag){
    int i;
    for(i = 0; i < tag->tags_count; i++){
        XDocument_DisposeTag(tag->tags[i]);
    }
    if(tag->type == TagType_Table) XDocument_DisposeTable((XDTable*)tag->Object);
    if(tag->type == TagType_Image) XDocument_DisposeImage((XDImage*)tag->Object);
    if(tag->type == TagType_Paragraph) XDocument_DisposeParagraph((XDParagraph*)tag->Object);
    //if(tag->type == TagType_Paragraph) XDocument_DisposeParagraph((XDParagraph*)tag->Object);
    //if(tag->name != NULL) free(tag->name);
    //if(tag->text != NULL) free(tag->text);
    //if(tag->Properties != NULL) free(tag->Properties);
    if(tag->tags != NULL) free(tag->tags);
    free(tag);
}

XDTag *XDocument_AddParagraph(char *buf, long *offset, char all){
    long seek = 0;
    XDTag *tag = (XDTag*)calloc(1, sizeof(XDTag));
    tag->type = TagType_Paragraph;
    int len = 0;
    memcpy(&len, buf, sizeof(int));
    seek += sizeof(int);
    XDParagraph *par = (XDParagraph*)calloc(1, sizeof(XDParagraph));
    par->text = (char*)malloc(len+1);
    memcpy(par->text, &buf[seek], len);
    par->text[len] = 0;
    seek += len;
    tag->Object = (void*)par;
    if(all){
        memcpy(&par->align, &buf[seek], sizeof(int));
        seek += sizeof(int);
        len = 0;
        memcpy(&len, &buf[seek], sizeof(int));
        seek += sizeof(int);
        if(len){
            par->href = (char*)malloc(len+1);
            memcpy(par->href, &buf[seek], len);
            par->href[len] = 0;
            seek += len;
        }
        len = 0;
        memcpy(&len, &buf[seek], sizeof(int));
        seek += sizeof(int);
        if(len){
            par->link = (char*)malloc(len+1);
            memcpy(par->link, &buf[seek], len);
            par->link[len] = 0;
            seek += len;
        }
        len = 0;
        memcpy(&len, &buf[seek], sizeof(int));
        seek += sizeof(int);
        if(len){
            par->isList = 1;
            par->listTxt = (char*)malloc(len+1);
            memcpy(par->listTxt, &buf[seek], len);
            par->listTxt[len] = 0;
            seek += len;
        }
        len = 0;
        memcpy(&len, &buf[seek++], 1);
        if(len) par->isTitle = 1;
    }
    else{
        par->align = XDAlign_Left;
        par->href = NULL;
        par->link = NULL;
        par->isList = 0;
        par->listTxt = NULL;
        par->isTitle = 0;
    }
    *offset += seek;
    return tag;
}

XDTag *XDocument_AddImage(char *buf, long *offset){
    long seek = 0;
    XDTag *tag = (XDTag*)calloc(1, sizeof(XDTag));
    tag->type = TagType_Image;
    XDImage *image = (XDImage*)calloc(1, sizeof(XDImage));
    int len = 0;
    tag->Object = (void*)image;
    memcpy(&len, buf, sizeof(int));
    seek += sizeof(int);
    if(len){
        image->fileName = (char*)malloc(len+1);
        memcpy(image->fileName, &buf[seek], len);
        image->fileName[len] = 0;
        seek += len;
    }
    memcpy(&image->width, &buf[seek], sizeof(int));
    seek += sizeof(int);
    memcpy(&image->height, &buf[seek], sizeof(int));
    seek += sizeof(int);
    memcpy(&image->align, &buf[seek], sizeof(int));
    seek += sizeof(int);
    *offset += seek;
    return tag;
}

XDTag *XDocument_AddTable(char *buf, long *offset){
    long seek = 0;
    XDTag *tag = (XDTag*)calloc(1, sizeof(XDTag));
    tag->type = TagType_Table;
    XDTable *table = (XDTable*)calloc(1, sizeof(XDTable));
    tag->Object = (void*)table;
    int len = 0;
    memcpy(&len, buf, sizeof(int));
    seek += sizeof(int);
    table->row_count = len;
    table->rows = (XDRow**)malloc(sizeof(XDRow*)*table->row_count);
    int i, j;
    for(i = 0; i < table->row_count; i++){
        XDRow *row = (XDRow*)malloc(sizeof(XDRow));
        len = 0;
        memcpy(&len, &buf[seek], sizeof(int));
        seek += sizeof(int);
        row->cells_count = len;
        row->height = 0;
        row->cells = (XDCell**)malloc(sizeof(XDCell*)*row->cells_count);
        for(j = 0; j < row->cells_count; j++){
            XDTag *ctag = (XDTag*)calloc(1, sizeof(XDTag));
            XDCell *cell = (XDCell*)malloc(sizeof(XDCell));
            ctag->Object = (void*)cell;
            cell->tag = ctag;
            int cell_data_len = 0;
            memcpy(&cell_data_len, &buf[seek], sizeof(int));
            seek += sizeof(int);
            long data_len = 0;
            char *data = &buf[seek];
            while(data_len < cell_data_len){
                TagsId tt = 0;
                XDTag *mtag = NULL;
                tt = data[data_len++];
                //memcpy(&tt, &data[data_len++], 1);
                if(tt == Paragraph){
                    mtag = XDocument_AddParagraph(&data[data_len], &data_len, 0);
                }
                else if(tt == Image){
                    mtag = XDocument_AddImage(&data[data_len], &data_len);
                }
                else break;
                ctag->tags = (XDTag**)realloc(ctag->tags, sizeof(XDTag*)*(ctag->tags_count+1));
                ctag->tags[ctag->tags_count++] = mtag;
            }
            seek += data_len;
            memcpy(&cell->width, &buf[seek], sizeof(int));
            seek += sizeof(int);
            memcpy(&cell->height, &buf[seek], sizeof(int));
            seek += sizeof(int);
            memcpy(&cell->valign, &buf[seek], sizeof(int));
            seek += sizeof(int);
            memcpy(&cell->vmerge, &buf[seek], sizeof(int));
            seek += sizeof(int);
            memcpy(&cell->hmerge, &buf[seek], sizeof(int));
            seek += sizeof(int);
            memcpy(&cell->span, &buf[seek], sizeof(int));
            seek += sizeof(int);
            row->cells[j] = cell;
            //if(row->height < cell->height) row->height = cell->height;
        }
        memcpy(&row->height, &buf[seek], sizeof(int));
        seek += sizeof(int);
        table->rows[i] = row;
        table->height += row->height;
    }
    memcpy(&table->height, &buf[seek], sizeof(int));
    seek += sizeof(int);
    *offset += seek;
    return tag;
}

void XDocument_AddContent(XDocument *doc, char *buf, long *offset){
    int len = 0;
    long seek = 0;
    memcpy(&len, buf, sizeof(int));
    seek += sizeof(int);
    doc->content = (XDContent*)malloc(sizeof(XDContent));
    doc->content->parts_count = len;
    doc->content->chapters = (XDChapter*)malloc(sizeof(XDChapter)*doc->content->parts_count);
    int i;
    for(i = 0; i < doc->content->parts_count; i++){
    //for(i = doc->content->parts_count; i >= 0; i--){
        memcpy(&len, &buf[seek], sizeof(int));
        seek += sizeof(int);
        doc->content->chapters[i].title = (char*)malloc(len + 1);
        doc->content->chapters[i].title[len] = 0;
        memcpy(doc->content->chapters[i].title, &buf[seek], len);
        seek += len;
        memcpy(&len, &buf[seek], sizeof(int));
        seek += sizeof(int);
        doc->content->chapters[i].link = (char*)malloc(len + 1);
        memcpy(doc->content->chapters[i].link, &buf[seek], len);
        doc->content->chapters[i].link[len] = 0;
        seek += len;
    }
    *offset += seek;
}

void XDocument_LoadBinDocument(XDocument *doc, char *content, long content_len){
    TagsId tagid = 0;
    long offset = 0;
    doc->tags = NULL;
    doc->tags_count = 0;
    while (offset < content_len) {
        tagid = content[offset++];
        switch(tagid){
        case Paragraph:
            XDocument_AddTag(doc, XDocument_AddParagraph(&content[offset], &offset, 1));
            break;
        case Image:
            XDocument_AddTag(doc, XDocument_AddImage(&content[offset], &offset));
            break;
        case Table:
            XDocument_AddTag(doc, XDocument_AddTable(&content[offset], &offset));
            break;
        case Content:
            XDocument_AddContent(doc, &content[offset], &offset);
        default: break;
        }
    }
}

void XDocument_LoadBinFile(XDocument *doc, char *fileName){
    FILE *f = fopen(fileName, "rb");
    if(f == NULL) return;
    doc->fileName = strdup(fileName);
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *fileContent = (char*)malloc(fsize);
    fread(fileContent, 1, fsize, f);
    fclose(f);
    XDocument_LoadBinDocument(doc, fileContent, fsize);
    free(fileContent);
    XDocument_SplitPages(doc);
}

void XDocument_AddTag(XDocument *doc, XDTag *tag){
    doc->tags = (XDTag**)realloc(doc->tags, sizeof(XDTag*)*(doc->tags_count+1));
    //memcpy(&doc->tags[doc->tags_count], tag, sizeof(XDTag));
    doc->tags[doc->tags_count++] = tag;
    //doc->tags_count++;
}

char** XDocument_Split(char *src, long src_len, char *splitter, int *count){
    int c = 0;
    int con = 0;
    long offset = 0;
    long seek = 0;
    char **result = NULL;
    char *sp = NULL;
    while((c = strstr(&src[offset], splitter) - &src[offset]) > 0){
        sp = (char*)calloc(c+1, 1);
        strncpy(sp, &src[offset], c);
        seek += c+1;
        result = (char**)realloc(result, sizeof(char*)*(con+1));
        result[con++] = sp;
        offset += c+1;
        if(offset >= src_len) break;
    }
    if(con>0 && offset < src_len){
        sp = (char*)calloc((src_len - offset)+1, 1);
        result = (char**)realloc(result, sizeof(char*)*(con+1));
        strncpy(sp, &src[offset], src_len-offset);
        result[con++] = sp;
    }
    *count = con;
    return result;
}

void XDocument_SplitPages(XDocument *doc){
    doc->pages_count = 0;
    doc->pages = NULL;
    //XDTag *tmp = doc->tags[1];
    //XDTag *tmp = doc->tags;
    XDTag **tmp = doc->tags;
    int i, j = 0;
    for(i = 0; i < doc->content->parts_count; i++){
        char *link = doc->content->chapters[i].link;
        XDPage *page = (XDPage*)malloc(sizeof(XDPage));
        page->link = link;
        page->tags = NULL;
        page->tags_count = 0;
        for(;j < doc->tags_count; j++){
            if(tmp[j]->type == TagType_Paragraph){
                XDParagraph *tpar = (XDParagraph*)tmp[j]->Object;
                if(tpar->link != NULL && strcmp(link, tpar->link) != 0){
                    if((i+1) < doc->content->parts_count && strcmp(tpar->link, doc->content->chapters[i+1].link) == 0){
                    break;
                    }
                }
            }
            page->tags = (XDTag**)realloc(page->tags, sizeof(XDTag*)*(page->tags_count+1));
            page->tags[page->tags_count++] = tmp[j];
        }
        doc->pages = (XDPage**)realloc(doc->pages, sizeof(XDPage*)*(doc->pages_count+1));
        doc->pages[doc->pages_count++] = page;
    }
}

