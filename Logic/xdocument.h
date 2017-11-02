#ifndef XDOCUMENT_H
#define XDOCUMENT_H

#include <X11/Xlib.h>

typedef enum XDAlign{
    XDAlign_Left,
    XDAlign_Right,
    XDAlign_Center,
    XDAlign_Inline
}XDAlign;

typedef enum XDVAlign{
    XDVAlign_Top,
    XDVAlign_Midle,
    XDVAlign_Bottom
}XDVAlign;

typedef enum TagType{
    TagType_Paragraph,
    TagType_Table,
    TagType_Image,
    TagType_Cell
}TagType;

typedef enum XDMerge{
    XDMerge_Start,
    XDMerge_None,
    XDMerge_Continue
}XDMerge;

//typedef enum XDListType{
//    XDList_Bullet,
//    XDList_Numeric
//}XDListType;



//typedef struct XDParagraphProperties{
//    XDAlign align;
//}XDParagraphProperties;

typedef struct XDChapter{
    char *title;
    char *link;
}XDChapter;

typedef struct XDContent{
    XDChapter *chapters;
    int parts_count;
}XDContent;

typedef struct XDTag{
    //char *name;
    //char *text;
    //long name_len;
    //long text_len;
    //void *Properties;
    //int prop_len;

    struct XDTag **tags;
    int tags_count;
    TagType type;
    void *Object;
}XDTag;

typedef struct XDCell{
    char *text;
    XDVAlign valign;
    XDMerge hmerge;
    XDMerge vmerge;
    int width;
    int height;
    int span;
    XDTag *tag;
}XDCell;

typedef struct XDRow{
    int height;
    XDCell **cells;
    int cells_count;
}XDRow;

typedef struct XDColumn{
    int width;
    XDCell **cells;
    int cells_count;
}XDColumn;

typedef struct XDTable{
    int width;
    int height;
    XDRow **rows;
    XDColumn **columns;
    int row_count;
    int col_count;
}XDTable;

typedef struct XDImage{
    int width;
    int height;
    XDAlign align;
    char *fileName;
    XImage *image;
}XDImage;

typedef struct XDParagraph{
    XDAlign align;
    char *text;
    char  *link;
    char *href;
    char isList;
    char *listTxt;
    char isTitle;
}XDParagraph;

typedef struct XDPage{
    XDTag **tags;
    int tags_count;
    char *link;
}XDPage;

typedef struct XDocument{
    XDTag **tags;
    int tags_count;

    char *fileName;
    char *doc;
    long doc_size;

    XDContent *content;
    XDPage **pages;
    int pages_count;
    Display *d;
}XDocument;

XDocument *XDocument_newDocument(Display *d);
void XDocument_LoadFile(XDocument *doc, char *fileName);
void XDocument_LoadDocument(XDocument *doc, char *doc_txt, long doc_size);
void XDocument_LoadBinFile(XDocument *doc, char *fileName);
void XDocument_Dispose(XDocument *doc);
XDTag *XDocument_GetTags(XDocument *doc, char *tag_name);
XDContent *XDocument_GetContent(XDocument *doc);
XDTag *XDocument_GetNextTag(char *doc, long *len);
void XDocument_DisposeTag(XDTag *tag);
char* XDocument_TrimTags(char *doc, long *len);
char** XDocument_Split(char *src, long src_len, char *splitter, int *count);

#endif // XDOCUMENT_H



