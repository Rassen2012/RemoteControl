#ifndef WIDGET_H
#define WIDGET_H

#include "graphics.h"

typedef enum WidgetDock{
    DOCK_LEFT,
    DOCK_LEFT_TOP,
    DOCK_LEFT_BOTTOM,
    DOCK_TOP,
    DOCK_BOTTOM,
    DOCK_RIGHT,
    DOCK_RIGHT_TOP,
    DOCK_RIGHT_BOTTOM,
    DOCK_CENTER,
    DOCK_STRACH,
    DOCK_TOP_STRACH,
    DOCK_BOTTOM_STRACH,
    DOCK_LEFT_STRACH,
    DOCK_RIGHT_STRACH
}WidgetDock;

typedef enum WidgetTypes{
    WIDGET_TYPE_FORM,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_TEXTBOX,
    WIDGET_TYPE_TEXTLINE,
    WIDGET_TYPE_LABEL,
    WIDGET_TYPE_LISTVIEW,
    WIDGET_TYPE_PANEL,
    WIDGET_TYPE_SPLITTER,
    WIDGET_TYPE_TREEVIEW,
    WIDGET_TYPE_TREEVIEW_ITEM,
    WIDGET_TYPE_TEXTVIEWER,
    WIDGET_TYPE_LISTVIEWITEM,
    WIDGET_TYPE_SCROLLBAR,
    WIDGET_TYPE_SLIDER,
    WIDGET_TYPE_DOCVIEW
}WidgetTypes;

typedef struct S_Widget{
    int x, y;
    int width, height;
    Color backgroundColor;
    Color foregroundColor;
    Pixmap backgroundImage;
    int type;
    void *object;

    struct S_Widget *parent;
    struct S_Widget **childs;
    Display *display;
    Window window;
    Graphics *g;

    char visible;
    char visibility;
    char focus;
    char hover;
    char is_tabed;
    char tab;
    char disposed;
    int tabIndex;
    int nextTab;
    int child_count;
    int dock;
}S_Widget;

S_Widget *Widget_newWidget(int x, int y, int width, int height, S_Widget *parent);
void Widget_Dispose(S_Widget *w);
void Widget_Show(S_Widget *w);
void Widget_Hide(S_Widget *w);
void Widget_Resize(S_Widget *w, int width, int height);
void Widget_ResizeEvent(S_Widget *w, ResizeEventArgs *ev);
void Widget_Tab(S_Widget *w);
void Widget_Untab(S_Widget *w);
void Widget_MousePressEvent(S_Widget *w, MouseEventArgs *ev);
void Widget_MouseReleaseEvent(S_Widget *w, MouseEventArgs *ev);
void Widget_MouseMoveEvent(S_Widget *w, MouseEventArgs *ev);
void Widget_MouseEnterEvent(S_Widget *w, MouseEventArgs *ev);
void Widget_MouseLeaveEvent(S_Widget *w, MouseEventArgs *ev);
void Widget_KeyPressEvent(S_Widget *w, KeyEventArgs *ev);
void Widget_KeyReleaseEvent(S_Widget *w, KeyEventArgs *ev);
void Widget_Move(S_Widget *w, int x, int y);

void Widget_SetBackgroundColor(S_Widget *w, Color c);
void Widget_SetForegroundColor(S_Widget *w, Color c);
void Widget_SetBorderColor(S_Widget *w, Color c);
void Widget_SetBackgroundImage(S_Widget *w, Pixmap image);

void Widget_AddChild(S_Widget *w, S_Widget *child);
void Widget_RemoveChild(S_Widget *w, S_Widget *child);
void Widget_Paint(S_Widget *w, PaintEventArgs *ev);
void Widget_GetTabIndex(S_Widget *w);
void Widget_SetDock(S_Widget *w, int dock);
void Widget_SetBorderWidth(S_Widget *w, int width);
void Widget_SetVisibility(S_Widget *w, char v);
void Widget_FocusIn(S_Widget *w, EventArgs *ev);
void Widget_FocusOut(S_Widget *w, EventArgs *ev);
void Widget_SetTabIndex(S_Widget *w, int index);
void Widget_ClickTab(S_Widget *w, int x, int y);
void Widget_NextTab(S_Widget *parent);
void Widget_PrevTab(S_Widget *parent);

#endif // WIDGET_H

