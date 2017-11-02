#include <stdio.h>
#include <stdlib.h>
#ifdef linux
#include <malloc.h>
#endif
#include <string.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
//#include <X11/Xutil.h>
#if defined(OC2K1x)
#include "button.h"
#include "general.h"
#include "system.h"
#else
#include "../Logic/system.h"
#endif
#include "form.h"
#include "panel.h"
#include <X11/Xutil.h>

#define FORM_BACGROUND_COLOR Color_GetColor(255, 255, 255)
#define FORM_FOREGROUND_COLOR Color_GetColor(0, 0, 0)

#if defined(OC2K1x)
#define HEAD_HEIGHT 20
#define CLOSE_BUTTON_SIZE 16
#endif

#if defined(OC2K1x)
#define CURSOR_HOV 5
#else
#define CURSOR_HOV 15
#endif

#define DEF_TITLE "Form"

void Form_AddEvent(Form *f, Event ev){
    pthread_mutex_lock(&f->evs_mutex);
    EventStack_Push(&f->evStack, ev);
    pthread_mutex_unlock(&f->evs_mutex);
}

EventStack *Form_GetEvent(Form *f){
    pthread_mutex_lock(&f->evs_mutex);
    EventStack *res = EventStack_Pop(&f->evStack);
    pthread_mutex_unlock(&f->evs_mutex);
    return res;
}

void Form_SetTab(Form *f, S_Widget *w){
    int i;
    for(i = 0; i < f->widget->child_count; i++){
        if(f->widget->childs[i] == w) Widget_Tab(w);
        else Widget_Untab(f->widget->childs[i]);
    }
}

void Form_SelectTab(Form *f){
    if(f->widget->child_count > 0){
        int i;
        for(i = 0; i < f->widget->child_count; i++){
            if(f->widget->childs[i]->tab){
                if(i + 1 == f->widget->child_count) break;
                Form_SetTab(f, f->widget->childs[i+1]);
                return;
            }
        }
        Form_SetTab(f, f->widget->childs[0]);
    }
}

void Form_ClickTab(Form *f, int x, int y){
    int i;
    for(i = 0; i < f->widget->child_count; i++){
        S_Widget *w = f->widget->childs[i];
        if(w->x < x && w->x + w->width > x && w->y < y && w->y + w->height > y){
            //Form_SetTab(f, w);
            Widget_ClickTab(w, x - w->x, y - w->y);
            break;
        }
    }
}

void Form_DrawBorder(Graphics *g, int width, int height){
    XSetForeground(g->widget->display, g->gc, BlackPixel(g->widget->display, DefaultScreen(g->widget->display)));
    XPoint p1, p2, p3, p4;
    p1.x = 0; p1.y = 0;
    p2.x = 0; p2.y = height - 1;
    p3.x = width - 1; p3.y = height - 1;
    p4.x = width - 1; p4.y = 0;
    Graphics_DrawLine(g, p1.x, p1.y, p2.x, p2.y);
    Graphics_DrawLine(g, p2.x, p2.y, p3.x, p3.y);
    Graphics_DrawLine(g, p3.x, p3.y, p4.x, p4.y);
    Graphics_DrawLine(g, p4.x, p4.y, p1.x, p1.y);
    XSetForeground(g->widget->display, g->gc, WhitePixel(g->widget->display, DefaultScreen(g->widget->display)));
    p1.x = 2; p1.y = 2;
    p2.x = 2; p2.y = height - 2;
    p3.x = width - 2; p3.y = height - 2;
    p4.x = width - 2; p4.y = 2;
    Graphics_DrawLine(g, p1.x, p1.y, p2.x, p2.y);
    Graphics_DrawLine(g, p2.x, p2.y, p3.x, p3.y);
    Graphics_DrawLine(g, p3.x, p3.y, p4.x, p4.y);
    Graphics_DrawLine(g, p4.x, p4.y, p1.x, p1.y);
}

void Form_Paint(Form *f, PaintEventArgs *ev){
    if(f->Paint != NULL){
        f->Paint(f, ev);
        return;
    }
    Graphics_BeginPaint(f->widget->g, ev);
    if(f->p != 0){
        XCopyArea(f->widget->display, f->p, f->widget->g->context, f->widget->g->gc, 0, 0, 300, 300, 0, 0);
    }
    else{
        Graphics_SetColor(f->widget->g, f->widget->backgroundColor);
        Graphics_FillRectangle(f->widget->g, 0, 0, f->widget->width, f->widget->height);
    }
    Form_DrawBorder(f->widget->g, f->widget->width, f->widget->height);
#if defined(OC2K1x)
    Graphics *g = f->widget->g;
    Graphics_SetColor(g, Color_GetColor1(40, 200, 40, g->widget->display));
    Graphics_FillRectangle(g, 0, 0, g->widget->width, HEAD_HEIGHT);
    int twidth = XTextWidth(g->fontInfo, f->title, strlen(f->title));
    Graphics_SetColor(g, BLACK_COLOR(g->widget->display));
    Graphics_DrawText(g, f->widget->width/2 - twidth/2, HEAD_HEIGHT - 2, f->title);
#endif
    Graphics_EndPaint(f->widget->g);
}

void Form_TmpResize(Form *f){
    XMoveResizeWindow(f->widget->display, f->widget->window, f->widget->x, f->widget->y, f->widget->width, f->widget->height);
    ResizeEventArgs *rev = newResizeEventArgs(f->widget->width, f->widget->height, f->widget->width, f->widget->height, f->widget->window);
    Widget_Resize(f->widget, f->widget->width, f->widget->height);
    Widget_ResizeEvent(f->widget, rev);
    free(rev);
}

void Form_ChangeCursor(Form *f, int cursor, FormCursors cur_cursor){
    if(f->currentCursor != FORM_DEFAULT_CURSOR){
        XUndefineCursor(f->widget->display, f->widget->window);
        XFreeCursor(f->widget->display, f->cur);
    }
    f->cur = XCreateFontCursor(f->widget->display, cursor);
    XDefineCursor(f->widget->display, f->widget->window, f->cur);
    f->currentCursor = cur_cursor;
}

void Form_MouseMove(Form *f, MouseEventArgs *ev){
    //XSetInputFocus(f->widget->display, f->widget->window, RevertToNone, CurrentTime);
//    ResizeEventArgs *rev;
//    if(f->rctrl && ev->button == MOUSE_BUTTON_LEFT){
//        f->widget->x += ev->global_x-ev->xg_prev;
//        f->widget->y += ev->global_y-ev->yg_prev;
//        XMoveWindow(f->widget->display, f->widget->window, f->widget->x, f->widget->y);
//        return;
//    }
#if defined(OC2K1x)
    if(f->headPressed){
        //f->widget->x += ev->global_x - ev->xg_prev;
        //f->widget->y += ev->global_y - ev->yg_prev;
        Form_Move(f, f->widget->x + (ev->global_x - ev->xg_prev), f->widget->y + (ev->global_y - ev->yg_prev));
    }
#endif
    if(f->MouseMove != NULL){
        f->MouseMove(f, ev);
    }
}

void Form_MousePress(Form *f, MouseEventArgs *ev){
#if defined(OC2K1x)
    if(f->widget->window == ev->win){
        XSetInputFocus(f->widget->display, f->widget->window, RevertToNone, CurrentTime);
        XRaiseWindow(f->widget->display, f->widget->window);
        if(ev->button == MOUSE_BUTTON_LEFT && ev->y < HEAD_HEIGHT) f->headPressed = 1;
    }
#endif
    if(f->MousePress != NULL){
        f->MousePress(f, ev);
    }
}

void Form_MouseRelease(Form *f, MouseEventArgs *ev){
#if defined(OC2K1x)
    if(ev->button == MOUSE_BUTTON_LEFT && f->headPressed) f->headPressed = 0;
#endif
    if(f->MouseRelease != NULL){
        f->MouseRelease(f, ev);
    }
}

void Form_KeyPress(Form *f, KeyEventArgs *ev){
    if(ev->ksym == XK_Control_L) f->rctrl = 1;
    if(f->KeyDown != NULL){
        f->KeyDown(f, ev);
    }
}

void Form_KeyRelease(Form *f, KeyEventArgs *ev){
    if(ev->ksym == XK_Control_L) f->rctrl = 0;
    if(f->KeyUp != NULL){
        f->KeyUp(f, ev);
    }
}

void Form_ResizeEvent(Form *f, ResizeEventArgs *ev){
    if(f->widget->window == ev->win){
        if(f->ResizeEvent != NULL){
            f->ResizeEvent(f, ev);
        }
    }
}

#if defined(OC2K1x)
void Form_CloseButtonClick(Button *b, void *arg){
    Form *f = (Form*)arg;
    Event ev;
    ev.func = NULL;
    ev.type = EV_CLOSE_WINDOW;
    Form_AddEvent(f, ev);
}
#endif

Form *Form_newForm(int x, int y, int width, int height, int type, Form *parent){

#if defined(OC2K1x)
  if(oc2kOClibsAccessInit(X11_MASK,1))
  {
    printff ("Error: oc2kOClibsAccessInit(X11_MASK,1)\n");
    return;
  }
#endif
    Form *f = (Form*)malloc(sizeof(Form));
    if(f == NULL) return NULL;
    f->parent = parent;
    f->type = type;
    f->visible = 0;
    f->Paint = NULL;
    f->min_height = 0;
    f->min_width = 0;
    f->rctrl = 0;
    f->obj = NULL;
    f->p = 0;
    f->evStack = NULL;
    f->title = DEF_TITLE;
    f->widget = Widget_newWidget(x, y, width, height, NULL);
    if(f->widget == NULL){
        free(f);
        return NULL;
    }
    f->widget->type = WIDGET_TYPE_FORM;
    f->widget->object = (void*)f;
    //f->widget->backgroundColor = FORM_BACGROUND_COLOR;
    //f->widget->foregroundColor = FORM_FOREGROUND_COLOR;
    XSelectInput(f->widget->display, f->widget->window, ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
                 EnterWindowMask | LeaveWindowMask | SubstructureNotifyMask | StructureNotifyMask | FocusChangeMask);

    f->MouseMove = NULL;
    f->MouseRelease = NULL;
    f->MousePress = NULL;
    f->KeyDown = NULL;
    f->KeyUp = NULL;
    f->ResizeEvent = NULL;
    f->MouseEnter = NULL;
    f->MouseLeave = NULL;
    f->Close = NULL;
    f->currentCursor = FORM_DEFAULT_CURSOR;
    f->mode = WINDOW_MODE;
    f->im = NULL;
    f->p = 0;
    f->max_width = f->max_height = f->min_width = f->min_height = 0;
    pthread_mutex_init(&f->evs_mutex, NULL);
#if defined(OC2K1x)
    f->btnClose = Button_newButton(f->widget->width - 18, 2, CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE, "X", f->widget);
    f->btnClose->arg = (void*)f;
    f->btnClose->Click = Form_CloseButtonClick;
    f->headPressed = 0;
#endif
    return f;
}

void Form_Dispose(Form *f){
    if(f == NULL) return;
    if(f->im != NULL) XDestroyImage(f->im);
    //pthread_mutex_lock(&f->evs_mutex);
    Widget_Dispose(f->widget);
    while(Form_GetEvent(f));
    //pthread_mutex_unlock(&f->evs_mutex);
    pthread_mutex_destroy(&f->evs_mutex);
    free(f);
}

void Form_Hide(Form *f){
    f->visible = 0;
    Widget_Hide(f->widget);
}

void Form_SetTitle(Form *f, const char *title){
    //memset(f->title, 0, sizeof(f->title));
    //strncpy(f->title, title, strlen(title));
    f->title = title;
    XStoreName(f->widget->display, f->widget->window, f->title);
}

void Form_SetMinSize(Form *f, int min_width, int min_height){
    f->min_width = min_width;
    f->min_height = min_height;
    XSizeHints sh;
    sh.min_width = min_width;
    sh.min_height = min_height;
    sh.flags = PMinSize;
    XSetWMNormalHints(f->widget->display, f->widget->window, &sh);
}

void Form_SetSizePolicy(Form *f, int min_width, int min_height, int max_width, int max_height){
    f->min_width = min_width;
    f->min_height = min_height;
    f->max_width = max_width;
    f->max_height = max_height;
    XSizeHints sh;
    sh.min_width = min_width;
    sh.min_height = min_height;
    sh.flags = PMinSize | PMaxSize;
    XSetWMNormalHints(f->widget->display, f->widget->window, &sh);
}

void Form_Move(Form *f, int x, int y){
    Widget_Move(f->widget, x, y);
}

void Form_Resize(Form *f, int width, int height){
    Widget_Resize(f->widget, width, height);
}

void Form_SetBackgroundColor(Form *f, Color c){
    f->widget->backgroundColor = c;
    XSetWindowBackground(f->widget->display, f->widget->window, c.l);
    Form_Paint(f, NULL);
}

void Form_SetForegroundColor(Form *f, Color c){
    f->widget->foregroundColor = c;
    Form_Paint(f, NULL);
}

void Form_SetBackgroundImage(Form *f, Pixmap image){
    f->widget->backgroundImage = image;
    Form_Paint(f, NULL);
}

int Form_X(Form *f){
    return f->widget->x;
}

int Form_Y(Form *f){
    return f->widget->y;
}

int Form_Width(Form *f){
    return f->widget->width;
}

int Form_Height(Form *f){
    return f->widget->height;
}

void Form_AddWidget(Form *f, S_Widget *child){
    Widget_AddChild(f->widget, child);
}

void Form_RemoveWidget(Form *f, S_Widget *child){
    Widget_RemoveChild(f->widget, child);
}

void Form_Close(Form *f){
    //Form_Dispose(f);
    if(f == NULL) return;
    if(f->Close != NULL) f->Close(f);
    Form_Dispose(f);
}

void Form_SetWindowMode(Form *f, FormMode mode){
    XWindowAttributes attr;
    f->mode = mode;
    //ResizeEventArgs *rev;
    XGetWindowAttributes(f->widget->display, f->widget->window, &attr);
#ifdef linux
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( f->widget->display, "_NET_WM_STATE", False );
    e.xclient.display = f->widget->display;
    e.xclient.window = f->widget->window;
    e.xclient.format = 32;
    e.xclient.data.l[0] = mode == FULLSCREEN_MODE ? 1 : 0;//b == True ? 1 : 0;
    e.xclient.data.l[1] = XInternAtom( f->widget->display, "_NET_WM_STATE_FULLSCREEN", False );
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;
    XSendEvent( f->widget->display, attr.root,
                False, SubstructureNotifyMask | SubstructureRedirectMask, &e );
    /*f->widget->width = attr.screen->width;
    f->widget->height = attr.screen->height;
    rev = newResizeEventArgs(f->widget->width, f->widget->height, f->widget->width, f->widget->height,
                             f->widget->window);
    Widget_ResizeEvent(f->widget, rev);
    free(rev);*/
#else
    ResizeEventArgs *rev;
    XMoveWindow(f->widget->display, f->widget->window, 0, 0);
    if(mode == FULLSCREEN_MODE){
    f->widget->width = attr.screen->width;
    f->widget->height = attr.screen->height;
    }
    else{
        f->widget->width = 700;
        f->widget->height = 500;
    }
    XResizeWindow(f->widget->display, f->widget->window, f->widget->width, f->widget->height);

    rev = newResizeEventArgs(f->widget->width, f->widget->height, f->widget->width, f->widget->height,
                             f->widget->window);
    Widget_ResizeEvent(f->widget, rev);
    Widget_Resize(f->widget, f->widget->width, f->widget->height);
    free(rev);
    Form_Paint(f, NULL);
#endif
}

void Form_SetWindowManagerHints(Display *d, char *pclass, char *argv[], int argc, Window win, int x, int y, int width, int height, int minWidth, int minHeight,
                                char *title, char *ititle, Pixmap pixmap, int max_width, int max_height){
    XSizeHints sizeHints;
    XWMHints wmHints;
    XClassHint classHint;
    XTextProperty windowname, iconname;
    if(!XStringListToTextProperty(&title, 1, &windowname) || !XStringListToTextProperty(&ititle, 1, &iconname)) return;
    sizeHints.flags = PPosition | PSize | PMinSize;
    sizeHints.min_width = minWidth;
    sizeHints.min_height = minHeight;
    sizeHints.width = width;
    sizeHints.height = height;
    if(max_width != 0 && max_height != 0){
    	sizeHints.max_width = max_width;
    	sizeHints.max_height = max_height;
	sizeHints.flags |= PMaxSize;
    }
    sizeHints.x = x;
    sizeHints.y = y;
    wmHints.flags = StateHint | IconPixmapHint | InputHint;
    wmHints.initial_state = NormalState;
    wmHints.input = True;
    wmHints.icon_pixmap = pixmap;
    classHint.res_name = argc == 0 ? 0 : argv[0];
    classHint.res_class = pclass;

    XSetWMProperties(d, win, &windowname, &iconname, argv, argc, &sizeHints, &wmHints, &classHint);
}

void Form_MouseEnter(Form *f){
#if defined (OC2K1x)
	if(f->currentCursor != FORM_DEFAULT_CURSOR){
		f->cur = XCreateFontCursor(f->widget->display, XC_left_ptr);
		XDefineCursor(f->widget->display, f->widget->window, f->cur);
		f->currentCursor = FORM_DEFAULT_CURSOR;
	}
#endif
	if(f->MouseEnter != NULL) f->MouseEnter(f);
}

void Form_MouseLeave(Form *f){
	if(f->MouseLeave != NULL) f->MouseLeave(f);
}

void Form_Show(Form *f){
    XEvent event;
    PaintEventArgs *pev = NULL;
    MouseEventArgs *mev = NULL;
    KeyEventArgs *kev = NULL;
    ResizeEventArgs *rev = NULL;
    EventArgs *ev = NULL;
    Atom wmDeleteMessage;
    EventStack *tmpEv;
    KeySym ksym;
    XImage *im;
    Panel *p;
    char ctrl_pressed = 0;

    Form_SetWindowManagerHints(f->widget->display, "window", 0, 0, f->widget->window, f->widget->x, f->widget->y, f->widget->width, f->widget->height,
                               f->min_width, f->min_height, f->title, f->title, 0, f->max_width, f->max_height);
#if defined(OC2K1x)
    XSetWindowAttributes swa;
    swa.override_redirect = True;
    XChangeWindowAttributes(f->widget->display, f->widget->window, CWOverrideRedirect, &swa);
    f->cur = XCreateFontCursor(f->widget->display, XC_left_ptr);
    XDefineCursor(f->widget->display, f->widget->window, f->cur);
    XRaiseWindow(f->widget->display, f->widget->window);
#endif

    Widget_Show(f->widget);

//    XSetWindowAttributes swa;
//    swa.override_redirect = True;
//    XChangeWindowAttributes(f->widget->display, f->widget->window, CWOverrideRedirect, &swa);
    //f->cur = XCreateFontCursor(f->widget->display, XC_left_ptr);
    //XDefineCursor(f->widget->display, f->widget->window, f->cur);



    char key[10];
    int x_prev, y_prev, xg_prev, yg_prev;
    x_prev = y_prev = xg_prev = yg_prev = -1;
    unsigned int btnPressed = 0;
    int n, depth = Color_GetDepth();
    //int pst;
    if(depth == 24) n = 32;
    else if(depth == 16) n = 16;
    else n = 8;
    //pst = 4;
    //Widget_SetBorderWidth(f->widget, 0);
    f->visible = 1;
    if(f->parent == NULL){
        wmDeleteMessage = XInternAtom(f->widget->display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(f->widget->display, f->widget->window, &wmDeleteMessage, 1);
        //unsigned long f_time = time_get();
        //int frame_per_sec = 0;
        while(f != NULL){
            while(XPending(f->widget->display)){
                XNextEvent(f->widget->display, &event);
                switch(event.type){
                case Expose:
                    pev = newPaintEventArgs(event.xexpose.x, event.xexpose.y, event.xexpose.width, event.xexpose.height);
                    if(pev == NULL) break;
                    pev->win = event.xexpose.window;
                    pev->con = 1;
                    Widget_Paint(f->widget, pev);
                    free(pev);
                    break;
                case KeyPress:
//                    if(event.xkey.keycode == KEY_ESC){
//                        //exit(0);
//                        //return;
//                        //if(f->im != NULL) XDestroyImage(f->im);
//                        Form_Close(f);
//                        return;
//                    }
                    //if(event.xkey.keycode == KEY_TAB){
                        //Widget_Tab(f->widget);
                        //break;
                        //Form_SelectTab(f);
                    //}

                    memset(key, 0, sizeof(key));
                    ksym = XLookupKeysym(&event.xkey, 0);
                    if(ksym == XK_Control_L) ctrl_pressed = 1;
                    else if((ksym == XK_E || ksym == XK_e) && ctrl_pressed){
                        Form_Close(f);
                        return;
                    }
                    printf("KeySym press - %d\n", (int)ksym);
                    printf("Key press - %d\n", event.xkey.keycode);
                    kev = newKeyEventArgs(event.xkey.keycode, '0', event.xkey.window);
                    if(kev == NULL) break;
                    if(event.xkey.state & ShiftMask) kev->shift_mod = 1;
                    if(event.xkey.state & LockMask) kev->caps_mod = 1;
                    if(event.xkey.state & ControlMask) kev->ctrl_mod = 1;
                    if(ksym == XK_Tab && !(event.xkey.state & ShiftMask)){
                        Widget_NextTab(f->widget);
                    }
                    else if(ksym == XK_Tab && (event.xkey.state & ShiftMask)){
                        Widget_PrevTab(f->widget);
                    }
                    kev->con = 1;
                    kev->keyChar = (char)ksym;
                    kev->ksym = ksym;
                    if(f->KeyDown) f->KeyDown(f, kev);
                    Widget_KeyPressEvent(f->widget, kev);
                    free(kev);
                    break;
                case KeyRelease:
                    kev = newKeyEventArgs(event.xkey.keycode, '0', event.xkey.window);
                    if(kev == NULL) break;
                    ksym = XLookupKeysym(&event.xkey, 0);
                    if(ksym == XK_Control_L) ctrl_pressed = 0;
                    kev->keyChar = (char)ksym;
                    kev->ksym = ksym;
                    kev->con = 1;
                    Widget_KeyReleaseEvent(f->widget, kev);
                    free(kev);
                    break;
                case EnterNotify:
                    mev = newMouseEventArgs(event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root,
                                            event.xbutton.button, event.xbutton.window);
                    if(mev == NULL) break;
                    mev->con = 1;
                    Widget_MouseEnterEvent(f->widget, mev);
                    free(mev);
                    break;
                case LeaveNotify:
                    mev = newMouseEventArgs(event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root,
                                            event.xbutton.button, event.xbutton.window);
                    if(mev == NULL) break;
                    mev->con = 1;
                    Widget_MouseLeaveEvent(f->widget, mev);
                    free(mev);
                    break;
                case ButtonPress:
                    //if(event.xbutton.window == f->widget->window) Form_ClickTab(f, event.xbutton.x, event.xbutton.y);
                    //else Form_ClickTab(f, event.xbutton.x_root - f->widget->x, event.xbutton.y_root - f->widget->y);
                    mev = newMouseEventArgs(event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root,
                                            event.xbutton.button, event.xbutton.window);
                    if(mev == NULL) break;
                    xg_prev = event.xbutton.x_root;
                    yg_prev = event.xbutton.y_root;
                    mev->con = 1;
                    btnPressed = event.xbutton.button;
                    Widget_MousePressEvent(f->widget, mev);
                    free(mev);
                    break;
                case ButtonRelease:
                    mev = newMouseEventArgs(event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root,
                                            event.xbutton.button, event.xbutton.window);
                    if(mev == NULL) break;
                    mev->con = 1;
                    btnPressed = 0;
                    Widget_MouseReleaseEvent(f->widget, mev);
                    free(mev);
                    break;
                case MotionNotify:
                    mev = newMouseEventArgs(event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root,
                                            event.xbutton.button, event.xbutton.window);
                    if(mev == NULL) break;
                    mev->con = 1;
                    mev->x_prev = x_prev;
                    mev->y_prev = y_prev;
                    mev->xg_prev = xg_prev;
                    mev->yg_prev = yg_prev;
                    mev->button = btnPressed;
                    Widget_MouseMoveEvent(f->widget, mev);
                    x_prev = mev->x;
                    y_prev = mev->y;
                    xg_prev = mev->global_x;
                    yg_prev = mev->global_y;
                    free(mev);
                    break;
                case FocusIn:
                    ev = newEventArgs(event.xfocus.window, NULL);
                    Widget_FocusIn(f->widget, ev);
                    free(ev);
                    break;
                case FocusOut:
                    ev = newEventArgs(event.xfocus.window, NULL);
                    Widget_FocusOut(f->widget, ev);
                    free(ev);
                    break;
                case ConfigureNotify:
                    //Resize window
                    //if(f->widget->width > event.xconfigure.width || f->widget->height > event.xconfigure.height) break;
#if defined(OC2K1x)
                    break;
#endif
                    if(f->widget->width == event.xconfigure.width && f->widget->height == event.xconfigure.height && f->widget->window == event.xconfigure.window) break;
                    if(f->widget->window != event.xconfigure.window) break;
                    rev = newResizeEventArgs(event.xconfigure.width, event.xconfigure.height, f->widget->width, f->widget->height,
                                             event.xconfigure.window);
                    if(rev == NULL) break;
                    rev->con = 1;
                    Form_Resize(f, event.xconfigure.width, event.xconfigure.height);
                    Widget_ResizeEvent(f->widget, rev);
                    free(rev);
                    f->widget->width = event.xconfigure.width;
                    f->widget->height = event.xconfigure.height;
                    break;
                case ClientMessage:
                    if((unsigned int)event.xclient.data.l[0] == wmDeleteMessage){
                        Form_Close(f);
                        return;
                        //exit(0);
                    }
                    break;
                }
            }
/*            if(time_get() - f_time > 1000){
                printf("frame per seconf - %d\n", frame_per_sec);
                frame_per_sec = 0;
                f_time = time_get();
            }*/
            while(1){//((tmpEv = EventStack_Pop(&f->evStack)) != NULL){
	    	//pthread_mutex_lock(&f->evs_mutex);
		tmpEv = Form_GetEvent(f);//EventStack_Pop(&f->evStack);
		//pthread_mutex_unlock(&f->evs_mutex);
		if(tmpEv == NULL) break;
                if(tmpEv->event.func != NULL){
                    tmpEv->event.func(tmpEv->event.arg);
                    continue;
                }
                switch(tmpEv->event.type){
                case EV_MOVE_PANEL:
                    p = (Panel*)tmpEv->event.arg;
                    Panel_Move(p, tmpEv->event.param, tmpEv->event.param1);
                    Form_Paint(f, NULL);
                    break;
                case EV_RAISE_WINDOW:
                    XRaiseWindow(f->widget->display, f->widget->window);
                    break;
                case EV_SET_PANEL:
                    break;
                case EV_SET_PANEL_IMAGE:
                    break;
                case EV_CLOSE_WINDOW:
                    Form_Close(f);
                    return;
                case EV_FORM_PAINT:
                    Form_Paint(f, NULL);
                    break;
                case EV_SET_IMAGE:
                  //  frame_per_sec++;
                    im = XCreateImage(f->widget->display, DefaultVisual(f->widget->display, DefaultScreen(f->widget->display)), Color_GetDepth(), ZPixmap, 0, (char*)tmpEv->event.arg, tmpEv->event.param,
                                      tmpEv->event.param1, n, 0);
                    if(im != NULL){
                        if(f->im != NULL) XDestroyImage(f->im);
                        f->im = im;
                        Form_Paint(f, NULL);
                    }
                    break;
                }
                free(tmpEv);
            }
            thread_pause(1);
        }
    }
    Form_Close(f);
}
