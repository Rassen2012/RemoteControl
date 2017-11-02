#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef linux
#include <malloc.h>
#include <X11/xpm.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <X11/Xlib.h>
#include <sched.h>
//#include <X11/xpm.h>
#include <X11/extensions/XTest.h>

//#include "general.h"
#if defined(OC2K1x)
#include "general.h"
#else
#include "system.h"
#endif

#include "minilzo/minilzo.h"
#include "shark.h"
#include "public.h"
//#include "Widgets/color.h"
#include "eventsqueue.h"
//#include "tmpclient.h"

#define COLORMAP_TIME 30000

typedef struct RCClient{
    ServerClientStates state;
    int servaddr;
    shark *sh;
    shark *ev_sh;
    shark *im_sh;
    VisualInfo vi;
    VisualInfo svi;
    Display *d;
    Window root;
    EventQueue *qu;
    XImage *im;
    pthread_mutex_t mutex;
    pthread_mutex_t ev_mutex;
    pthread_t si_th;
    pthread_t th;
    pthread_t main_th;
    char *buf;
    char map_change;
    char full_image;
    int step;
    float mouse_x, mouse_y;
    char serv_active;
    int asx, asy;
    char *hname;

    int serv_addr;
    unsigned short cm[256][3];
    unsigned char cur_cm[256][3];

    char thi_close;
    char th_close;
    char thm_close;
}RCClient;

RCClient client = {0};

void RCClient_SetVisualInfo();
void RCCLient_MainCicle();
void RCClient_Receive(int addr, char *buf, int size);
void RCClient_ScaleImage(int oldw, int oldh, int neww, int newh, char *src, char *dst, int n);
void RCClient_CompressImage(char *dst, unsigned long dst_size, char *src, unsigned long *src_size);
void RCClient_SendScreen(shark *sh, int width, int height);
void RCClient_SetEvent(int addr, char *buf, int size);
void *RCClient_SendThread(void *arg);
void *RCClient_SendImageThread(void *arg);
int RCClient_ScaleImage1(int width, int height, int nwidth, int nheight, char *src, char *dst, int n);
//int RCClient_ScaleImage2(int width, int height, int nwidth, int nheight, char *src, char *dst, int n);

void RCClient_CreateMousePressEvent(int x, int y, int btn);
void RCClient_CreateMouseMoveEvent(int x, int y);
void RCClient_CreateMouseReleaseEvent(int x, int y, int btn);
void RCClient_CreateKeyPressEvent(KeySym ksym);
void RCClient_CreateKeyReleaseEvent(KeySym ksym);

void RCClient_RleCompress2(char *old, char *src, int src_size, char *dst, int *dst_size, int n, int lx){
    int i, s;
    //unsigned int tmp1, tmp2;
    //tmp1 = tmp2 = 0;
    //lx = (lx>>1);
    for(i = 0, s = 0; i < src_size; i+=lx){
        if(memcmp(&src[i], &old[i], lx) != 0){
            dst[s++] = 1;
            memcpy(&dst[s], &src[i], lx);
            s+=lx;
        }
        else{
            dst[s++] = 0;
        }
        if(s > src_size) return;
    }
    *dst_size = s;
}

void RCClient_RleCompress3(char *old, char *src, int src_size, char *dst, int *dst_size, int n){
    int i, s = 0;
    unsigned char sym_count = 0;
    unsigned int cur, next, ol;
    unsigned int *it_i, *it_io;
    unsigned short *it_s, *it_so;
    unsigned char *it_c, *it_co;
    size_t step = n;
    *dst_size = 0;
    if(src_size < 1) return;
    cur = next = ol = 0;
    if(step == 2){
        it_s = (unsigned short*)src;
        it_so = (unsigned short*)old;
        cur = *it_s++;
        ol = *it_so++;
        for(i = 0; i < src_size;){
            if(cur == ol){
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 0;
                        dst[s++] = sym_count;
                        sym_count = 0;
                    }
                }while((i+=step) < src_size && (next = *it_s++) == (ol = *it_so++));
                if(sym_count > 0){
                    dst[s++] = 0;
                    dst[s++] = sym_count;
                    sym_count = 0;
                }
            }
            else{
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 1;
                        dst[s++] = sym_count;
                        memcpy(&dst[s], &cur, step);
                        s+=step;
                        sym_count = 0;
                    }
                    it_so++;
                }while((i+=step) < src_size && (next = *it_s++) == cur);
                if(sym_count > 0){
                    dst[s++] = 1;
                    dst[s++] = sym_count;
                    memcpy(&dst[s], &cur, step);
                    s+=step;
                    sym_count = 0;
                }
                ol = *(it_so-1);
            }
            cur = next;
            if(s >= src_size) return;
        }
    }
    else if(step == 1){
        it_c = (unsigned char*)src;
        it_co = (unsigned char*)old;
        cur = *it_c++;
        ol = *it_co++;
        for(i = 0; i < src_size;){
            if(cur == ol){
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 0;
                        dst[s++] = sym_count;
                        sym_count = 0;
                    }
                }while(++i < src_size && (next = *it_c++) == (ol = *it_co++));
                if(sym_count > 0){
                    dst[s++] = 0;
                    dst[s++] = sym_count;
                    sym_count = 0;
                }
            }
            else{
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 1;
                        dst[s++] = sym_count;
                        dst[s++] = cur;
                        sym_count = 0;
                    }
                    it_co++;
                }while(++i < src_size && (next = *it_c++) == cur);
                if(sym_count > 0){
                    dst[s++] = 1;
                    dst[s++] = sym_count;
                    dst[s++] = cur;
                    sym_count = 0;
                }
                ol = *(it_co-1);
            }
            cur = next;
            if(s >= src_size) return;
        }
    }
    else if(step == 4){// || step == 3){
        it_i = (unsigned int*)src;
        it_io = (unsigned int*)old;
        cur = *it_i++;
        ol = *it_io++;
        for(i = 0; i < src_size;){
            if(cur == ol){
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 0;
                        dst[s++] = sym_count;
                        sym_count = 0;
                    }
                }while((i+=step) < src_size && (next = *it_i++) == (ol = *it_io++));
                if(sym_count > 0){
                    dst[s++] = 0;
                    dst[s++] = sym_count;
                    sym_count = 0;
                }
            }
            else{
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 1;
                        dst[s++] = sym_count;
                        memcpy(&dst[s], &cur, step);
                        s+=step;
                        sym_count = 0;
                    }
                    it_io++;
                }while((i+=step) < src_size && (next = *it_i++) == cur);
                if(sym_count > 0){
                    dst[s++] = 1;
                    dst[s++] = sym_count;
                    memcpy(&dst[s], &cur, step);
                    s+=step;
                    sym_count = 0;
                }
                ol = *(it_io-1);
            }
            cur = next;
            if(s >= src_size) return;
        }
    }
    else if(step == 3){
        memcpy(&cur, src, step);
        memcpy(&ol, old, step);
        for(i = 0; i < src_size;){
            if(cur == ol){
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 0;
                        dst[s++] = sym_count;
                        sym_count = 0;
                    }
                    i+=step;
                    if(i >= src_size) break;
                    //memcpy(&next, &src[i], step);
                    //memcpy(&ol, &old[i], step);
                }while(memcmp(&src[i], &old[i], step) == 0);//(next == ol);
                memcpy(&next, &src[i], step);
                memcpy(&ol, &old[i], step);
                if(sym_count > 0){
                    dst[s++] = 0;
                    dst[s++] = sym_count;
                    sym_count = 0;
                }
            }
            else{
                do{
                    sym_count++;
                    if(sym_count == 255){
                        dst[s++] = 1;
                        dst[s++] = sym_count;
                        memcpy(&dst[s], &cur, step);
                        s+=step;
                        sym_count = 0;
                    }
                    i+=step;
                    if(i >= src_size) break;
                    //memcpy(&next, &src[i], step);
                }while(memcmp(&src[i], &cur, step) == 0);//(next == cur);
                memcpy(&next, &src[i], step);
                if(sym_count > 0){
                    dst[s++] = 1;
                    dst[s++] = sym_count;
                    memcpy(&dst[s], &cur, step);
                    s+=step;
                    sym_count = 0;
                }
                memcpy(&ol, &old[i], step);
            }
            cur = next;
            if(s >= src_size) return;
        }
    }
    else{
        return;
    }
    *dst_size = s;
}

void RCClient_RleCompress1(char *old, char *src, int src_size, char *dst, int *dst_size, int n){
    int i, j, f, t, s = 0;
    int sym_count = 0;
    char max_char = 127;
    unsigned int cur, next, ol;
    unsigned int cm = 0x80000000;
    //int ss_count = 0;
    size_t step = n;
    *dst_size = 0;
    if(src_size < 1) return;

    memcpy(&cur, src, step);
    if(old != NULL) memcpy(&ol, old, step);
    for(i = step; i < src_size;){
        sym_count = 0;
        if(old == NULL || cur != ol){
            do{
                sym_count++;
                memcpy(&next, &src[i], step);
                i+=step;
            }while(i < src_size && next == cur);
            if(sym_count > max_char){
                j = sym_count/max_char;
                f = sym_count%max_char;
                //j+=f;
                for(t = 0; t < j; t++){
                    dst[s++] = max_char;
                    memcpy(&dst[s], &cur, step);
                    s+=step;
                }
                if(f > 0){
                    dst[s++] = (char)f;
                    memcpy(&dst[s], &cur, step);
                    s+=step;
                }
            }
            else{
                dst[s++] = (char)sym_count;
                memcpy(&dst[s], &cur, step);
                s+=step;
            }
            if(old != NULL) memcpy(&ol, &old[i-step], step);
        }
        else{
            do{
                sym_count++;
                memcpy(&next, &src[i], step);
                memcpy(&ol, &old[i], step);
                i+=step;
            }while(i < src_size && next == ol);
            if(sym_count > max_char){
                j = sym_count/max_char;
                f = sym_count%max_char;
                for(t = 0; t < j; t++){
                    dst[s++] = max_char;
                    memcpy(&dst[s], &cm, step);
                    s+=step;
                }
                if(f > 0){
                    dst[s++] = (char)f;
                    memcpy(&dst[s], &cm, step);
                    s+=step;
                }
            }
            else{
                dst[s++] = (char)sym_count;
                memcpy(&dst[s], &cm, step);
                s+=step;
            }
        }
        cur = next;
        if(s >= src_size){
            return;
        }
    }
    //printf("SS count - %d\n", ss_count);
    *dst_size = s;
}

void RCClient_RleCompress(char *src, int src_size, char *dst, int *dst_size){
    int i, s = 0;
    //int j, f, n;
    int sym_count = 0;
    unsigned int cur, next;
    //char max_char = 127;
    size_t step = sizeof(unsigned int);
    *dst_size = 0;
    if(src_size < 0) return;
    memcpy(&cur, src, sizeof(int));
    for(i = step; i < src_size;){
        sym_count = 0;
        do{
            sym_count++;
            memcpy(&next, &src[i], step);
            i+=step;
        }while(i < src_size && next == cur);
        //if(sym_count > max_char){
        //j = sym_count/max_char;
        //f = sym_count%max_char;
        //            j+=f;
        memcpy(&dst[s], &sym_count, step);
        s+=step;
        memcpy(&dst[s], &cur, step);
        s+=step;
        //            for(n = 0; n < j; n++){
        //                dst[s++] = max_char;
        //                memcpy(&dst[s], &cur, step);
        //                s+=step;
        //            }
        //            if(f > 0){
        //                dst[s++] = (char)f;
        //                memcpy(&dst[s], &cur, step);
        //                s+=step;
        //            }
        //        }
        //        else{
        //            dst[s++] = (char)sym_count;
        //            memcpy(&dst[s], &cur, step);
        //            s+=step;
        //        }
        if(s >= src_size) {
            return;
        }
        cur = next;
    }
    *dst_size = s;
}

void RCClient_GetColormap(){
    //Color_SetDepth(attr.depth, client.d);
    //if(attr.depth != 8) return;
    //char buf[SHARK_BUFFSIZE] = {0};
    //unsigned short cm[256][3];// = {0};
    XColor col;
    short cm[256][3];// = {0};
    Colormap colm = XDefaultColormap(client.d, XDefaultScreen(client.d));
    int i;
    for(i = 0; i < 256; i++){
        memset(&col, 0, sizeof(XColor));
        col.pixel = i;
        XQueryColor(client.d, colm, &col);
        cm[i][0] = col.red;
        cm[i][1] = col.green;
        cm[i][2] = col.blue;
        client.cur_cm[i][0] = cm[i][0]>>8;
        client.cur_cm[i][1] = cm[i][1]>>8;
        client.cur_cm[i][2] = cm[i][2]>>8;
    }
    /*if(memcmp(cm, client.cm, sizeof(client.cm)) != 0){
        memcpy(client.cm, cm, sizeof(client.cm));
        return 1;
    }
    else{
        return 0;
    }*/
    pthread_mutex_lock(&client.mutex);
    if(memcmp(cm, client.cm, sizeof(client.cm)) != 0){
        memcpy(client.cm, cm, sizeof(client.cm));
        client.map_change = 1;
    }
    pthread_mutex_unlock(&client.mutex);
    //return &cm;
    //memcpy(client.cm, cm, sizeof(cm));
}

void RCClient_SetVisualInfo(){
#if defined(OC2K1x)
    if(oc2kOClibsAccessInit(X11_MASK,1))
    {
        printff ("Error: oc2kOClibsAccessInit(X11_MASK,1)\n");
        return;
    }
#endif
    XWindowAttributes attr;
    client.svi.screen_height = 0;
    client.svi.screen_width = 0;
    client.d = XOpenDisplay(NULL);
    client.root = XRootWindow(client.d, DefaultScreen(client.d));
    XGetWindowAttributes(client.d, client.root, &attr);
    client.vi.depth = attr.depth;
    client.vi.screen_height = attr.height;
    client.vi.screen_width = attr.width;
    printf("Screen width - %d, screen height - %d\n", client.vi.screen_width, client.vi.screen_height);
    double res = (double)client.vi.screen_width/(double)client.vi.screen_height;
    char ok = 0;
    int i, j;
    for(i = 1; i < 30; i++){
        for(j = 1; j < 30; j++){
            if((double)i/(double)j == res){
                ok = 1;
                break;
            }
        }
        if(ok) break;
    }
    client.asx = i;
    client.asy = j;
    printf("Aspect ratio is %d:%d\n", client.asx, client.asy);
    client.mouse_x = client.mouse_y = 0;
    switch(client.vi.depth){
    case 24:
        client.step = 4;
        break;
    case 16:
        client.step = 2;
        break;
    default:
        client.step = 1;
        break;
    }
    //Color_SetDepth(attr.depth, client.d);
    if(attr.depth != 8) return;
    //char buf[SHARK_BUFFSIZE] = {0};
    unsigned short cm[256][3];// = {0};
    XColor col;
    Colormap colm = XDefaultColormap(client.d, XDefaultScreen(client.d));
    for(i = 0; i < 256; i++){
        memset(&col, 0, sizeof(XColor));
        col.pixel = i;
        XQueryColor(client.d, colm, &col);
        cm[i][0] = col.red;
        cm[i][1] = col.green;
        cm[i][2] = col.blue;
        client.cur_cm[i][0] = cm[i][0]>>8;
        client.cur_cm[i][1] = cm[i][1]>>8;
        client.cur_cm[i][2] = cm[i][2]>>8;
    }
    memcpy(client.cm, cm, sizeof(cm));
}

void RCClient_MainCicle(){
    Events evn;
    KeyEvent *kev;
    MouseEvent *mev;
    int res;
    unsigned long time_pal = time_get();
    unsigned int md;
    int x, y, x1, y1;
    Window w1, w2;
    while(client.state != STATE_NONE){
        while(1){
            pthread_mutex_lock(&client.ev_mutex);
            res = EventsQueue_Pop(&client.qu, &evn);
            pthread_mutex_unlock(&client.ev_mutex);
            if(res < 1) break;
            switch(evn.type){
            case EVENT_TYPE_KEY:
                kev = (KeyEvent*)evn.event;
                switch(kev->event){
                case KEY_PRESS:
                    RCClient_CreateKeyPressEvent(kev->keySym);
                    break;
                case KEY_RELEASE:
                    RCClient_CreateKeyReleaseEvent(kev->keySym);
                    break;
                }
                free(kev);
                break;
            case EVENT_TYPE_MOUSE:
                mev = (MouseEvent*)evn.event;
                switch(mev->event){
                case MOUSE_MOVE:
                    RCClient_CreateMouseMoveEvent(mev->x, mev->y);
                    break;
                case MOUSE_PRESS:
                    RCClient_CreateMousePressEvent(mev->x, mev->y, mev->button);
                    break;
                case MOUSE_RELEASE:
                    RCClient_CreateMouseReleaseEvent(mev->x, mev->y, mev->button);
                    break;
                default:
                    break;
                }
                free(mev);
            }
        }
        XQueryPointer(client.d, client.root, &w1, &w2, &x, &y, &x1, &y1, &md);
        pthread_mutex_lock(&client.mutex);
        client.mouse_x = ((float)x*100.0)/(float)client.vi.screen_width;
        client.mouse_y = ((float)y*100.0)/(float)client.vi.screen_height;
        if(client.im != NULL) {
            XDestroyImage(client.im);
        }
        client.im = XGetImage(client.d, client.root, 0, 0, client.vi.screen_width, client.vi.screen_height, AllPlanes, ZPixmap);
        pthread_mutex_unlock(&client.mutex);
        if(time_get() - time_pal > COLORMAP_TIME){
            RCClient_GetColormap();
            time_pal = time_get();
        }
        thread_pause(30);
    }
}

void *RCClient_MainThread(void *arg){
    client.thm_close = 0;
    RCClient_SetVisualInfo();
    pthread_mutex_init(&client.mutex, NULL);
    pthread_mutex_init(&client.ev_mutex, NULL);
    client.sh = shark_init(SERVER_PORT, CLIENT_PORT, BROADCAST_ADDR, RCClient_Receive);
#if defined OC2K1x
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = 130;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&client.th, &attr, RCClient_SendThread, NULL);
#else
    pthread_create(&client.th, NULL, RCClient_SendThread, NULL);
#endif
    RCClient_MainCicle();
    //pthread_mutex_destroy(&client.mutex);
    client.thm_close = 1;
    return NULL;
}

void RCClient_Start(char *hname){
    if(client.state != STATE_NONE) return;
    client.hname = strdup(hname);
    client.state = STATE_PASSIVE;
    client.thi_close = 1;
    client.thm_close = 1;
    client.th_close = 1;
    client.servaddr = 0;
    client.full_image = 0;
    client.im = NULL;
#if defined OC2K1x
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = 110;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&client.main_th, &attr, RCClient_MainThread, NULL);
#else
    pthread_create(&client.main_th, NULL, RCClient_MainThread, NULL);
#endif
}

void RCClient_Stop(){
    if(client.state == STATE_NONE) return;
    client.state = STATE_NONE;
    //pthread_join(client.th, NULL);
    //pthread_cancel(client.sh->thrserver);
    //pthread_join(client.sh->thrserver, NULL);
    //close(client.sh->sockrcv);
    //close(client.sh->socksend);
    //pthread_cancel(client.main_th);
    //pthread_join(client.si_th, NULL);

    while(!client.thi_close || !client.th_close || !client.thm_close) thread_pause(1);

    pthread_join(client.th, NULL);
    pthread_join(client.main_th, NULL);

    shark_close(client.sh);
    pthread_mutex_destroy(&client.ev_mutex);
    pthread_mutex_destroy(&client.mutex);
    //free(client.sh);
}

void RCClient_SendColormap(shark *sh){
    char buf[SHARK_BUFFSIZE] = {0};
    //unsigned short cm[256][3] = {0};
    //XColor col;
    //Colormap colm = XDefaultColormap(client.d, XDefaultScreen(client.d));
    int size, com;//, i;
    //    for(i = 0; i < 256; i++){
    //        memset(&col, 0, sizeof(XColor));
    //        col.pixel = i;
    //        XQueryColor(client.d, colm, &col);
    //        cm[i][0] = col.red;
    //        cm[i][1] = col.green;
    //        cm[i][2] = col.blue;
    //    }
    size = sizeof(int)*2 + sizeof(client.cm);
    com = C_GET_COLORMAP;
    memcpy(buf, &size, sizeof(int));
    memcpy(&buf[sizeof(int)], &com, sizeof(int));
    memcpy(&buf[sizeof(int)*2], client.cm, sizeof(client.cm));
    shark_send(sh, buf, size);
    //memcpy(client.cm, cm, sizeof(cm));
}

void RCClient_Receive(int addr, char *buf, int size){
    int com, tmp;
    ImageInfo imInfo;
    shark *sh;
    int loc_addr = 0;
    if(size < (int)sizeof(int)) return;
    memcpy(&tmp, buf, sizeof(int));
    if(tmp < size) return;
    memcpy(&com, &buf[sizeof(int)], sizeof(int));
    switch(com){
    case C_GET_SCREEN:
        memcpy(&imInfo, &buf[sizeof(int)*2], sizeof(ImageInfo));
        loc_addr = shark_getLocalAddr();
        printf("Loc addr - %d\n", loc_addr);
        sh = shark_init(SERVER_RECV_PORT+loc_addr, CLIENT_PORT_1+loc_addr, addr, NULL);
        RCClient_SendScreen(sh, imInfo.width, imInfo.height);
        //close(sh->sockrcv);
        //close(sh->socksend);
        //free(sh);
        shark_close(sh);
        break;
    case C_SET_ACTIVE_STATE:
        if(client.state == STATE_ACTIVE || !client.thi_close){
            thread_pause(1000);
            if(client.state == STATE_ACTIVE || !client.thi_close) break;
        }
        memcpy(&tmp, &buf[sizeof(int)*2], sizeof(int));
        client.state = STATE_ACTIVE;
        client.servaddr = addr;
        client.ev_sh = shark_init(tmp, CLIENT_ACTIVE_PORT, addr, RCClient_SetEvent);
#if defined OC2K1x
        pthread_attr_t attr;
        struct sched_param param;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        pthread_attr_getschedparam(&attr, &param);
        param.sched_priority = 90;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&client.si_th, &attr, RCClient_SendImageThread, NULL);
#else
        pthread_create(&client.si_th, NULL, RCClient_SendImageThread, NULL);
#endif
        break;
    case C_SET_PASSIVE_STATE:
        if(client.state == STATE_PASSIVE) break;
        client.state = STATE_PASSIVE;
        pthread_join(client.si_th, NULL);
        //pthrad_cancel(client.si_th);
        //pthread_cancel(client.ev_sh->thrserver);
        //pthread_join(client.ev_sh->thrserver, NULL);
        //close(client.ev_sh->socksend);
        //close(client.ev_sh->sockrcv);
        //free(client.ev_sh);
        //shark_close(client.ev_sh);
        break;
    case C_GET_COLORMAP:
        sh = shark_init(SERVER_RECV_PORT, CLIENT_PORT, addr, NULL);
        RCClient_SendColormap(sh);
        //close(sh->sockrcv);
        //close(sh->socksend);
        //free(sh);
        shark_close(sh);
        break;
    }
}

void RCClient_SetEvent(int addr, char *buf, int size){
    int type;//, tm;
    MouseEvent *mev;
    KeyEvent *kev;
    Events evn;
    if(size < (int)sizeof(int) || addr != client.servaddr) return;
    client.serv_active = 1;
    memcpy(&type, buf, sizeof(int));
    switch(type){
    case EVENT_TYPE_MOUSE:
        mev = (MouseEvent*)malloc(sizeof(MouseEvent));
        memcpy(mev, &buf[sizeof(int)], sizeof(MouseEvent));
        mev->x = ((float)mev->x/100.0)*(float)client.vi.screen_width;
        mev->y = ((float)mev->y/100.0)*(float)client.vi.screen_height;
        evn.type = EVENT_TYPE_MOUSE;
        evn.event = (void*)mev;
        pthread_mutex_lock(&client.ev_mutex);
        EventsQueue_Push(&client.qu, evn);
        pthread_mutex_unlock(&client.ev_mutex);
        break;
    case EVENT_TYPE_KEY:
        kev = (KeyEvent*)malloc(sizeof(KeyEvent));
        memcpy(kev, &buf[sizeof(int)], sizeof(KeyEvent));
        evn.type = EVENT_TYPE_KEY;
        evn.event = (void*)kev;
        pthread_mutex_lock(&client.ev_mutex);
        EventsQueue_Push(&client.qu, evn);
        pthread_mutex_unlock(&client.ev_mutex);
        break;
    case C_SET_VISUAL_INFO:
        memcpy(&client.svi, &buf[sizeof(int)], sizeof(VisualInfo));
        if(client.svi.screen_width == client.vi.screen_width && client.svi.screen_height == client.vi.screen_height) break;
        if((double)client.svi.screen_width/(double)client.svi.screen_height > (double)client.vi.screen_width/(double)client.vi.screen_height){
            client.svi.screen_width = (client.vi.screen_width*client.svi.screen_height)/client.vi.screen_height;
        }
        else{
            client.svi.screen_height = (client.svi.screen_width*client.vi.screen_height)/client.vi.screen_width;
        }
        break;
    case C_SET_PASSIVE_STATE:
        //        printf("Set passive!\n");
        //client.state = STATE_PASSIVE;
        //pthread_cancel(client.si_th);
        client.state = STATE_PASSIVE;
        //pthread_join(client.si_th, NULL);
        //pthread_cancel(client.ev_sh->thrserver);
        //shark_close(client.ev_sh);
        //client.state = STATE_PASSIVE;
        //free(client.ev_sh);
        break;
    case C_GET_FULL_IMAGE:
        client.full_image = 1;
        break;
    default:
        break;
    }
}

void RCClient_SendImage(char *buf, int size, int width, int height, char rle, int cur_height){
    int size_tmp, size_send, cur_pack;
    ImageInfo imInfo;
    char sh_buf[SHARK_BUFFSIZE] = {0};
    imInfo.width = width;
    imInfo.height = height;
    imInfo.size = size;
    imInfo.depth = client.vi.depth;
    imInfo.rle = rle;
    imInfo.cur_height = cur_height;
    cur_pack = size_send = 0;
    //    if(!imInfo.rle){
    //        memcpy(sh_buf, &cur_pack, sizeof(int));
    //        memcpy(&sh_buf[sizeof(int)], &imInfo, sizeof(ImageInfo));
    //        shark_send(client.ev_sh, sh_buf, sizeof(int)+sizeof(ImageInfo));
    //        shark_send_all(client.ev_sh, buf, size);
    //        return;
    //    }
    while(size_send < size){
        if(cur_pack == 0){
            size_tmp = size > (int)(SHARK_BUFFSIZE-sizeof(int)-sizeof(ImageInfo)) ? (int)(SHARK_BUFFSIZE-(sizeof(int)+sizeof(ImageInfo))) : size;
            memcpy(sh_buf, &cur_pack, sizeof(int));
            memcpy(&sh_buf[sizeof(int)], &imInfo, sizeof(ImageInfo));
            memcpy(&sh_buf[sizeof(int)+sizeof(ImageInfo)], buf, size_tmp);
            size_send = size_tmp;
            size_tmp = sizeof(int)+sizeof(ImageInfo)+size_send;
        }
        else{
            size_tmp = (size-size_send) > (int)(SHARK_BUFFSIZE - sizeof(int)) ? (int)(SHARK_BUFFSIZE - sizeof(int)) : (size-size_send);
            memcpy(sh_buf, &cur_pack, sizeof(int));
            memcpy(&sh_buf[sizeof(int)], &buf[size_send], size_tmp);
            size_send += size_tmp;
            size_tmp += sizeof(int);//+size_tmp;
        }

        //if(shark_send(client.ev_sh, sh_buf, size_tmp) < size_tmp) break;
        while(shark_send(client.ev_sh, sh_buf, size_tmp) < size_tmp) thread_pause(1);

        //if(size_send > sizeof(int)) printf("Send pack\n!");
        cur_pack++;
        //if(!rle) thread_pause(1);
    }
}

void RCClient_SendNull(){
    char buf[sizeof(int)] = {0};
    shark_send(client.ev_sh, buf, sizeof(int));
}

void RCClient_SendPalete(){
    char buf[SHARK_BUFFSIZE] = {0};
    int com = C_SET_COLORMAP;
    memcpy(buf, &com, sizeof(int));
    pthread_mutex_lock(&client.mutex);
    memcpy(&buf[sizeof(int)], client.cm, sizeof(client.cm));
    pthread_mutex_unlock(&client.mutex);
    shark_send(client.ev_sh, buf, sizeof(client.cm)+sizeof(int));
}

void RCClient_SendMousePoint(){
    char buf[SHARK_BUFFSIZE] = {0};
    int com = C_SET_MOUSE_COORD;
    memcpy(buf, &com, sizeof(int));
    pthread_mutex_lock(&client.mutex);
    memcpy(&buf[sizeof(int)], &client.mouse_x, sizeof(int));
    memcpy(&buf[sizeof(int)*2], &client.mouse_y, sizeof(int));
    pthread_mutex_unlock(&client.mutex);
    shark_send(client.ev_sh, buf, sizeof(int)*3);
}

void *RCClient_SendImageThread(void *arg){
    int tmp_res;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &tmp_res);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &tmp_res);
    char *imageBuf, *tmpBuf, *oldImageBuf = NULL;
    char *oldBuf = NULL;
    int image_size;
    unsigned long src_size, dst_size, tt;
    unsigned long col_time;
    int client_width, client_height;
    int n, tmp;
    int cur_height;
    char rle;
    if(client.vi.depth == 24) n = 4;
    else if(client.vi.depth == 16) n = 2;
    else n = 1;
    thread_pause(1000);
    while(client.svi.screen_height == 0 || client.svi.screen_width == 0) thread_pause(1);
    //tt = time_get();
    client_width = client_height = 0;
    col_time = time_get();
    if(client.vi.depth == 8) {
        RCClient_SendPalete();
        thread_pause(500);
    }
    client.serv_active = 1;
    client.thi_close = 0;
    while(client.state == STATE_ACTIVE){
        //tt = time_get();
        if(client_width != client.svi.screen_width || client_height != client.svi.screen_height || client.full_image){
            client.full_image = 0;
            if(oldImageBuf != NULL){
                free(oldImageBuf);
                oldImageBuf = NULL;
            }
            if(oldBuf != NULL){
                free(oldBuf);
                oldBuf = NULL;
            }
        }
        if(client.vi.depth == 8 && client.map_change){
            client.map_change = 0;
            RCClient_SendPalete();
            thread_pause(500);
            if(oldImageBuf != NULL){
                free(oldImageBuf);
                oldImageBuf = NULL;
            }
            if(oldBuf != NULL){
                free(oldBuf);
                oldBuf = NULL;
            }
        }
        if(time_get() - col_time > TIME_WAIT){
            if(!client.serv_active){
                printf("Breake from cicle! Time - %d\n", (int)(time_get()-col_time));
                break;
            }
            client.serv_active = 0;
            col_time = time_get();
        }
        client_width = client.svi.screen_width;
        client_height = client.svi.screen_height;
        image_size = client.vi.screen_height*client.vi.screen_width*n;
        //image_size = client_width*client_height*n;
        imageBuf = (char*)malloc(sizeof(char)*image_size);

        pthread_mutex_lock(&client.mutex);
        memcpy(imageBuf, client.im->data, image_size);
        //cur_width = client.im->bytes_per_line;
        pthread_mutex_unlock(&client.mutex);

        if(oldImageBuf == NULL || memcmp(oldImageBuf, imageBuf, image_size) != 0){
            if(oldImageBuf != NULL) free(oldImageBuf);
            oldImageBuf = (char*)malloc(sizeof(char)*image_size);
            memcpy(oldImageBuf, imageBuf, sizeof(char)*image_size);
            if(client.vi.screen_height != client_height || client.vi.screen_width != client_width){
                //tt = time_get();
                int tmp_size = client_width*client_height*n;
                //if(n == 3) n = 1;
                if(n==1) {tmp_size *= 4;};// n*=3;}
                tmpBuf = (char*)malloc(sizeof(char)*tmp_size);
                memset(tmpBuf, 0, tmp_size);
                //tmpBuf = (char*)malloc(sizeof(char)*client_width*client_height*n);
                tt = time_get();
                //cur_height = RCClient_ScaleImage1(client.vi.screen_width, client.vi.screen_height, client_width, client_height, imageBuf, tmpBuf, n);
                cur_height = RCClient_ScaleImage2(client.vi.screen_width, client.vi.screen_height, client_width, client_height, imageBuf, tmpBuf, n);
                tt = time_get() - tt;
                printf("Time to scale image - %d\n", (int)tt);
                free(imageBuf);
                imageBuf = tmpBuf;
                if(n==1) n = 4;
                //tt = time_get() - tt;
                //printf("Time to scale image - %d\n", (int)tt);
            }
            else {
                cur_height = client_height;
            }
            src_size = client_width*client_height*n;
            dst_size = src_size;
            tmpBuf = (char*)malloc(sizeof(char)*dst_size);
            //tt = time_get();
            //RCClient_RleCompress(imageBuf, src_size, tmpBuf, &dst_size);
            tmp = (int)dst_size;
            //            if(time_get() - tt > 4000 && oldBuf != NULL){
            //                free(oldBuf);
            //                oldBuf = NULL;
            //                tt = time_get();
            //            }
            //RCClient_RleCompress1(oldBuf, imageBuf, (int)src_size, tmpBuf, &tmp, n);
            //if(oldBuf != NULL) RCClient_RleCompress2(oldBuf, imageBuf, (int)src_size, tmpBuf, &tmp, n, n * client_width);
            if(oldBuf != NULL) {
                tt = time_get();
                RCClient_RleCompress3(oldBuf, imageBuf, (int)src_size, tmpBuf, &tmp, n);
                //RCClient_RleCompress2(oldBuf, imageBuf, (int)src_size, tmpBuf, &tmp, n, n*client_width);
                tt = time_get() - tt;
                printf("Time to rle compress - %d\n", (int)tt);
            }
            else tmp = 0;
            //tmp = 0;
            if(n == 4 && client.vi.depth == 8 && (client.vi.screen_height != client_height || client.vi.screen_width != client_width)) n = 1;
            dst_size = tmp;
            if(oldBuf != NULL) free(oldBuf);
            oldBuf = (char*)malloc(src_size);
            memcpy(oldBuf, imageBuf, src_size);
            //tt = time_get() - tt;
            //printf("Time rle compress - %d\n", (int)tt);
            if(dst_size > 0){
                rle = 1;
                free(imageBuf);
                imageBuf = tmpBuf;
                src_size = dst_size;
                tmpBuf = (char*)malloc(sizeof(char)*dst_size);
            }
            else{
                rle = 0;
                dst_size = src_size;
            }
            RCClient_CompressImage(imageBuf, src_size, tmpBuf, &dst_size);
            free(imageBuf);
            imageBuf = tmpBuf;
            RCClient_SendImage(imageBuf, dst_size, client_width, client_height, rle, cur_height);
            //printf("Send count - %d\n", ++a);
            free(imageBuf);
        }
        else{
            free(imageBuf);
            RCClient_SendNull();
        }
        RCClient_SendMousePoint();
        thread_pause(10);
    }
    if(oldBuf != NULL) free(oldBuf);
    if(oldImageBuf != NULL) free(oldImageBuf);
    shark_close(client.ev_sh);
    client.state = STATE_PASSIVE;
    client.thi_close = 1;
    return NULL;
}

void RCClient_SendScreen(shark *sh, int width, int height){
    int size_send, com, n, ret;
    unsigned long ss;
    char *buf, *tmpBuf;
    //unsigned long tt;
    ImageInfo imInfo;
    int imageSize = client.vi.screen_height*client.vi.screen_width*client.step;
    if(client.im == NULL) return;
    char *imageBuf = (char*)malloc(sizeof(char)*imageSize);
    pthread_mutex_lock(&client.mutex);
    memcpy(imageBuf, client.im->data, imageSize);
    pthread_mutex_unlock(&client.mutex);
    if(width >= client.vi.screen_width || height >= client.vi.screen_height){
        imInfo.width = client.vi.screen_width;
        imInfo.height = client.vi.screen_height;
    }
    else{
        imInfo.width = width;
        imInfo.height = height;
        imageSize = width*height*client.step;
        if(client.step == 1) imageSize *= 4;
        tmpBuf = (char*)malloc(sizeof(char)*imageSize);
        imInfo.cur_height = RCClient_ScaleImage2(client.vi.screen_width, client.vi.screen_height, width, height, imageBuf, tmpBuf, client.step);
        //imageSize = imInfo.width*imInfo.height*client.step;
        //RCClient_TmpScaleImage(client.vi.screen_width, client.vi.screen_height, width, height, imageBuf, tmpBuf, client.step);
        //scale_frame1(imageBuf, tmpBuf, 0, 0, width*client.step, width, client.vi.screen_width, client.vi.screen_height);
        free(imageBuf);
        imageBuf = tmpBuf;
    }
    imInfo.depth = client.vi.depth;
    com = C_GET_SCREEN;

    tmpBuf = (char*)malloc(sizeof(char)*imageSize);
    ss = imageSize;
    RCClient_CompressImage(imageBuf, ss, tmpBuf, &ss);
    free(imageBuf);

    imInfo.size = ss;
    size_send = sizeof(int)+sizeof(ImageInfo);
    buf = (char*)malloc(sizeof(char)*size_send);
    memcpy(buf, &com, sizeof(int));
    memcpy(&buf[sizeof(int)], &imInfo, sizeof(ImageInfo));
    ret = shark_send(sh, buf, size_send);
    free(buf);
    if(ret < size_send){
        return;
    }
    size_send = imInfo.size;
    buf = shark_read(sh, 0, &n);
    if(n < 1){
        printf("Send image failed, first\n");
        free(tmpBuf);
        return;
    }
    memcpy(&n, buf, sizeof(int));
    if(n == RECEIVE_FAILED){
        free(tmpBuf);
        printf("Send image failed, second!\n");
        return;
    }

    n = shark_send_all(sh, tmpBuf, size_send);
    free(tmpBuf);
    if(n != size_send){
        printf("Send image failed, send just %d\n", n);
    }
    else{
        printf("Send image succes!\n");
    }
}

int RCClient_GetFixedColor(unsigned char r, unsigned char g, unsigned char b){
    int i, index, val1, val2, val3;
    index = 0;
    val3 = -1;
    val2 = 0;
    val1 = (r << 16) | (g << 8) | b;
    for(i = 0; i < 256; i++){
        val2 = ((unsigned char)(client.cm[i][0]>>8) << 16) | ((unsigned char)(client.cm[i][1]>>8) << 8) | (unsigned char)(client.cm[i][2]>>8);
        if(abs(val1 - val2) < val3 || val3 < 0){
            index = i;
            val3 = abs(val1 - val2);
            if(val3 == 0) break;
        }
    }
    return index;
}

int RCClient_Scale24(int dx, int height, int dstdx, int nheight, char *src_loc, char *dst){
    int line=0;
    unsigned char *src = (unsigned char*)src_loc;

    {
        int ksrc[dx],i,j,k,up=0,*pksrc=ksrc;
        memset (ksrc,1,sizeof (ksrc));
        /* Находим области перехода на следующий пиксел исходника, выровненный по целому краю */
        for (i=0;i<dx;i++,pksrc++)
        {
            /* Добавляем в числитель размер пиксела приемника */
            up+=dstdx;
            /* Если суммарный остаток превысил размер исходного пиксела */
            if (up<dx)
            {
                /* Делаем переход на следующий пиксел */
                *pksrc=0;
            }
            else
                /* Вычисляем остаток пиксела */
                up-=dx;
        }
        /* Сюда будут копиться ргб пикселов. */
        int rgb[dstdx][3],*prgb;
        /* Массив пикселей источника в текущей строке приемника */
        int count[dstdx],*pcount;
        memset (rgb,0,sizeof(rgb));
        memset (count,0,sizeof (count));
        /* Для глубины 8 бит */
        {
            unsigned char *dstl=dst;
            int v = 0;
            // Используя массив, начинаем сжатие
            for (i=0;i<height;i++)
            {
                // Пробегаемся по очередной строке исходников
                for (pcount=count,prgb=(int *)rgb,j=0;j<dx;j++)
                {
                    //unsigned char color=*src;
                    prgb[2]+=src[v++];//client.cur_cm[color][0];
                    prgb[1]+=src[v++];//client.cur_cm[color][1];
                    prgb[0]+=src[v++];//client.cur_cm[color][2];
                    v++;
                    (*pcount)++;
                    if (ksrc[j])
                    {
                        pcount++;
                        prgb+=3;
                    }
                }
                //printf ("222\n");

                /* Как только мы накопили строку приемника, ее нужно подготовить */
                if (ksrc[i])
                {
                    /* Если сжатие более, чем в два раза */
                    if ((dstdx<<1)<dx)
                    {
                        // Скидываем накопленную строку приемника
                        for (k=0;k<dstdx;k++)
                        {
                            // Нормируем цвет поделив сумму на общую площадь результирующего пиксела
                            // Заносим апроксимацию цвета в приемник
                            dstl[3] = 255;
                            dstl[2]=rgb[k][0]/count[k];
                            dstl[1]=rgb[k][1]/count[k];
                            dstl[0]=rgb[k][2]/count[k];
                            dstl+=4;
                        }
                    }
                    /* Сжатие в два и менее раз */
                    else
                    {
                        // Скидываем накопленную строку приемника
                        for (k=0;k<dstdx;k++)
                        {
                            /* Определяем, на сколько делить. Если 1 пиксел, то не делим, 2- делим на 2, 4-на четыре*/
                            int kk=count[k]>>1;
                            // Нормируем цвет поделив сумму на общую площадь результирующего пиксела
                            // Заносим апроксимацию цвета в приемник
                            dstl[3] = 0;
                            dstl[2]=rgb[k][0]>>kk;
                            dstl[1]=rgb[k][1]>>kk;
                            dstl[0]=rgb[k][2]>>kk;
                            dstl+=4;
                        }
                    }
                    // Зануляем для очередного накопления
                    memset (rgb,0,sizeof (rgb));
                    memset (count,0,sizeof (count));
                    line++;
                    if (line>=nheight) break;
                }
            }
        }
        return line;
    }
    return 0;
}

int RCClient_Scale8 (int dx,int height,int dstdx,int nheight,char *src,char *dst)
{
    int line=0;

    {
        int ksrc[dx],i,j,k,up=0,*pksrc=ksrc;
        memset (ksrc,1,sizeof (ksrc));
        /* Находим области перехода на следующий пиксел исходника, выровненный по целому краю */
        for (i=0;i<dx;i++,pksrc++)
        {
            /* Добавляем в числитель размер пиксела приемника */
            up+=dstdx;
            /* Если суммарный остаток превысил размер исходного пиксела */
            if (up<dx)
            {
                /* Делаем переход на следующий пиксел */
                *pksrc=0;
            }
            else
                /* Вычисляем остаток пиксела */
                up-=dx;
        }
        /* Сюда будут копиться ргб пикселов. */
        int rgb[dstdx][3],*prgb;
        /* Массив пикселей источника в текущей строке приемника */
        int count[dstdx],*pcount;
        memset (rgb,0,sizeof(rgb));
        memset (count,0,sizeof (count));
        /* Для глубины 8 бит */
        {
            unsigned char *dstl=dst;
            // Используя массив, начинаем сжатие
            for (i=0;i<height;i++)
            {
                // Пробегаемся по очередной строке исходников
                for (pcount=count,prgb=(int *)rgb,j=0;j<dx;j++,src++)
                {
                    unsigned char color=*src;
                    prgb[0]+=client.cur_cm[color][0];
                    prgb[1]+=client.cur_cm[color][1];
                    prgb[2]+=client.cur_cm[color][2];
                    (*pcount)++;
                    if (ksrc[j])
                    {
                        pcount++;
                        prgb+=3;
                    }
                }
                //printf ("222\n");

                /* Как только мы накопили строку приемника, ее нужно подготовить */
                if (ksrc[i])
                {
                    /* Если сжатие более, чем в два раза */
                    if ((dstdx<<1)<dx)
                    {
                        // Скидываем накопленную строку приемника
                        for (k=0;k<dstdx;k++)
                        {
                            // Нормируем цвет поделив сумму на общую площадь результирующего пиксела
                            // Заносим апроксимацию цвета в приемник
                            dstl[3] = 255;
                            dstl[2]=rgb[k][0]/count[k];
                            dstl[1]=rgb[k][1]/count[k];
                            dstl[0]=rgb[k][2]/count[k];
                            dstl+=4;
                        }
                    }
                    /* Сжатие в два и менее раз */
                    else
                    {
                        // Скидываем накопленную строку приемника
                        for (k=0;k<dstdx;k++)
                        {
                            /* Определяем, на сколько делить. Если 1 пиксел, то не делим, 2- делим на 2, 4-на четыре*/
                            int kk=count[k]>>1;
                            // Нормируем цвет поделив сумму на общую площадь результирующего пиксела
                            // Заносим апроксимацию цвета в приемник
                            dstl[3] = 255;
                            dstl[2]=rgb[k][0]>>kk;
                            dstl[1]=rgb[k][1]>>kk;
                            dstl[0]=rgb[k][2]>>kk;
                            dstl+=4;
                        }
                    }
                    // Зануляем для очередного накопления
                    memset (rgb,0,sizeof (rgb));
                    memset (count,0,sizeof (count));
                    line++;
                    if (line>=nheight) break;
                }
            }
        }
        return line;
    }
    return 0;
}


int RCClient_ScaleImage2(int width, int height, int nwidth, int nheight, char *src, char *dst, int n){
    int x, y, index, in;
    int a_b, a_r, a_g, b_b, b_r, b_g, c_b, c_r, c_g, d_b, d_r, d_g;
    float x_ratio = ((float)(width-1))/nwidth;
    float y_ratio = ((float)(height-1))/nheight;
    float x_diff, y_diff, blue, red, green;
    float x_diff_r, y_diff_r;
    unsigned char tal = 255;
    //int offset = 0;
    int i, j, s = 0;
    float XDxYD, XDRxYDR, XDRxYD, XDxYDR;
    if (n==1 && nwidth <= width && nheight <= height)// && (nwidth<<1)>=width)
    {
        return RCClient_Scale8 (width,height,nwidth,nheight,src,dst);
        //return;
    }
    if(n == 1 && (nwidth > width || nheight > height)){
        return RCClient_ScaleImage1(width, height, nwidth, nheight, src, dst, n);
    }

    if(n == 4 && (nwidth <= width && nheight <= height)){
        return RCClient_Scale24(width, height, nwidth, nheight, src, dst);
    }

    if(n == 4){
        unsigned int *it_src = (unsigned int*)src;
        unsigned int *it_dst = (unsigned int*)dst;
        for(i = 0; i < nheight; i++){
            for(j = 0; j < nwidth; j++){
                x = (int)(x_ratio * j);
                y = (int)(y_ratio * i);
                x_diff = (x_ratio * j) - x;
                y_diff = (y_ratio * i) - y;
                index = y*width+x;

                a_b = it_src[index]&0xff;
                a_g = (it_src[index]>>8)&0xff;
                a_r = (it_src[index]>>16)&0xff;

                index++;
                b_b = it_src[index]&0xff;
                b_g = (it_src[index]>>8)&0xff;
                b_r = (it_src[index]>>16)&0xff;

                --index;
                index = index + width;
                c_b = it_src[index]&0xff;
                c_g = (it_src[index]>>8)&0xff;
                c_r = (it_src[index]>>16)&0xff;

                index++;
                d_b = it_src[index]&0xff;
                d_g = (it_src[index]>>8)&0xff;
                d_r = (it_src[index]>>16)&0xff;

                x_diff_r = 1-x_diff;
                y_diff_r = 1-y_diff;

                XDxYD = x_diff*y_diff;
                XDRxYDR = x_diff_r*y_diff_r;
                XDRxYD = x_diff_r*y_diff;
                XDxYDR = x_diff*y_diff_r;

                blue = a_b*XDRxYDR + b_b*XDxYDR + c_b*XDRxYD + d_b*XDxYD;
                green = a_g*XDRxYDR + b_g*XDxYDR + c_g*XDRxYD + d_g*XDxYD;
                red = a_r*XDRxYDR + b_r*XDxYDR + c_r*XDRxYD + d_r*XDxYD;

                *it_dst++ = (tal<<24) | ((unsigned char)red)<<16 | ((unsigned char)green)<<8 | (unsigned char)blue;
            }
        }
    }
    else if(n == 2){
        unsigned short *it_s = (unsigned short*)src;
        unsigned short *it_sd = (unsigned short*)dst;
        for(i = 0; i < nheight; i++){
            for(j = 0; j < nwidth; j++){
                x = (int)(x_ratio * j);
                y = (int)(y_ratio * i);
                x_diff = (x_ratio * j) - x;
                y_diff = (y_ratio * i) - y;
                index = y*width+x;

                a_b = it_s[index]&0x1f;
                a_g = (it_s[index]>>5)&0x3f;
                a_r = (it_s[index]>>11)&0x1f;

                index++;
                b_b = it_s[index]&0x1f;
                b_g = (it_s[index]>>5)&0x3f;
                b_r = (it_s[index]>>11)&0x1f;

                index--;
                index+=width;
                c_b = it_s[index]&0x1f;
                c_g = (it_s[index]>>5)&0x3f;
                c_r = (it_s[index]>>11)&0x1f;

                index++;
                d_b = it_s[index]&0x1f;
                d_g = (it_s[index]>>5)&0x3f;
                d_r = (it_s[index]>>11)&0x1f;

                x_diff_r = 1-x_diff;
                y_diff_r = 1-y_diff;

                XDxYD = x_diff*y_diff;
                XDRxYDR = x_diff_r*y_diff_r;
                XDRxYD = x_diff_r*y_diff;
                XDxYDR = x_diff*y_diff_r;

                blue = a_b*XDRxYDR + b_b*XDxYDR + c_b*XDRxYD + d_b*XDxYD;
                green = a_g*XDRxYDR + b_g*XDxYDR + c_g*XDRxYD + d_g*XDxYD;
                red = a_r*XDRxYDR + b_r*XDxYDR + c_r*XDRxYD + d_r*XDxYD;

                *it_sd++ = ((unsigned char)red<<11) | (((unsigned char)green&0x3f)<<5) | ((unsigned char)blue&0x1f);
            }
        }
    }
    else if(n == 1){
        //unsigned char *it_dst = (unsigned char*)dst;
        float ta, tb;
        for(i = 0; i < nheight; i++){
            tb = y_ratio*i;
            y = (int)tb;
            y_diff = tb-y;
            y_diff_r = 1-y_diff;
            for(j = 0; j < nwidth; j++){
                ta = x_ratio*j;
                //x = (int)(x_ratio * j);
                //y = (int)(y_ratio * i);
                //x_diff = (x_ratio * j) - x;
                //y_diff = (y_ratio * i) - y;
                x = (int)ta;
                //y = (int)tb;
                x_diff = ta-x;
                //y_diff = tb-y;
                index = y*width+x;

                in = src[index];
                //a = ((client.cm[in][0]>>8)<<16) | ((client.cm[in][1]>>8)<<8) | (client.cm[in][2]>>8);
                a_b = client.cur_cm[in][2];//client.cm[in][2]>>8;
                a_g = client.cur_cm[in][1];//client.cm[in][1]>>8;
                a_r = client.cur_cm[in][0];//client.cm[in][0]>>8;
                index++;
                in = src[index];
                //b = ((client.cm[in][0]>>8)<<16) | ((client.cm[in][1]>>8)<<8) | (client.cm[in][2]>>8);
                b_b = client.cur_cm[in][2];//client.cm[in][2]>>8;
                b_g = client.cur_cm[in][1];//client.cm[in][1]>>8;
                b_r = client.cur_cm[in][0];//client.cm[in][0]>>8;
                --index;
                index = index + width;
                in = src[index];
                //c = ((client.cm[in][0]>>8)<<16) | ((client.cm[in][1]>>8)<<8) | (client.cm[in][2]>>8);
                c_b = client.cur_cm[in][2];//client.cm[in][2]>>8;
                c_g = client.cur_cm[in][1];//client.cm[in][1]>>8;
                c_r = client.cur_cm[in][0];//client.cm[in][0]>>8;
                index++;
                in = src[index];
                //d = ((client.cm[in][0]>>8)<<16) | ((client.cm[in][1]>>8)<<8) | (client.cm[in][2]>>8);
                d_b = client.cur_cm[in][2];//client.cm[in][2]>>8;
                d_g = client.cur_cm[in][1];//client.cm[in][1]>>8;
                d_r = client.cur_cm[in][0];//client.cm[in][0]>>8;

                x_diff_r = 1-x_diff;
                //y_diff_r = 1-y_diff;

                XDxYD = x_diff*y_diff;
                XDRxYDR = x_diff_r*y_diff_r;
                XDRxYD = x_diff_r*y_diff;
                XDxYDR = x_diff*y_diff_r;

                /**it_dst++ = a_b*XDRxYDR + b_b*XDxYDR + c_b*XDRxYD + d_b*XDxYD;
                *it_dst++ = a_g*XDRxYDR + b_g*XDxYDR + c_g*XDRxYD + d_g*XDxYD;
                *it_dst++ = a_r*XDRxYDR + b_r*XDxYDR + c_r*XDRxYD + d_r*XDxYD;
                *it_dst++ = 0;
*/
                blue = a_b*XDRxYDR + b_b*XDxYDR + c_b*XDRxYD + d_b*XDxYD;
                green = a_g*XDRxYDR + b_g*XDxYDR + c_g*XDRxYD + d_g*XDxYD;
                red = a_r*XDRxYDR + b_r*XDxYDR + c_r*XDRxYD + d_r*XDxYD;
                /*
            blue = a_b*x_diff_r*y_diff_r + b_b*x_diff*y_diff_r + c_b*y_diff*x_diff_r + d_b*x_diff*y_diff;
            green = a_g*x_diff_r*y_diff_r + b_g*x_diff*y_diff_r + c_g*y_diff*x_diff_r + d_g*x_diff*y_diff;
            red = a_r*x_diff_r*y_diff_r + b_r*x_diff*y_diff_r + c_r*y_diff*x_diff_r + d_r*x_diff*y_diff;
            */
                /*blue = (a&0xff)*(x_diff_r)*(y_diff_r) + (b&0xff)*x_diff*(y_diff_r) +
                (c&0xff)*y_diff*(x_diff_r) + (d&0xff)*x_diff*y_diff;
            green = ((a>>8)&0xff)*(x_diff_r)*(y_diff_r) + ((b>>8)&0xff)*x_diff*(y_diff_r) +
                ((c>>8)&0xff)*y_diff*(x_diff_r) + ((d>>8)&0xff)*x_diff*y_diff;
            red = ((a>>16)&0xff)*(x_diff_r)*(y_diff_r) + ((b>>16)&0xff)*x_diff*(y_diff_r) +
                ((c>>16)&0xff)*y_diff*(x_diff_r) + ((d>>16)&0xff)*x_diff*y_diff;
            */

                dst[s++] = blue;
                dst[s++] = green;
                dst[s++] = red;
                dst[s++] = 0;
                //*it_dst++ = blue;
                //*it_dst++ = green;
                //*it_dst++ = red;
                //*it_dst++ = 0;
                //dst[s++] = 0;
            }
        }
    }
    return nheight;
}

int RCClient_ScaleImage1(int width, int height, int nwidth, int nheight, char *src, char *dst, int n){
    //printf ("scaleimage1 width=%d height=%d nwidth=%d nheight=%d\n",width,height,nwidth,nheight);
#if 1
    unsigned int colors[256];
    int upx,upy=0;
    int i,j;
    unsigned char *cursrc=src;
    unsigned int *curdst;
    unsigned char tal = 255;
    int deltaw=nwidth-width,deltah=nheight-height,sizeline=nwidth<<2;
    /* Создаем индексный массив 4байтных цветов */
    for (i=0;i<256;i++)
        colors[i]=client.cur_cm[i][2]|(client.cur_cm[i][1]<<8)|(client.cur_cm[i][0]<<16);
    /* Пробегаемся по всем строкам*/
    for (j=0;j<height;j++)
    {
        curdst=(unsigned int*)dst;
        /* Пробегаемся по всем столбцам */
        upx=0;
        int flg_second=0;
        for (i=0;i<width;i++,cursrc++)
        {
            //printf ("i=%d j=%d width=%d nwidth=%d height=%d nheight=%d curdst-save=%d\n",i,j,width,nwidth,height,nheight,(void *)curdst-(void *)savedst);
            /* Если предыдущий был пиксел повторитель */
#if 1
            if (flg_second)
            {
                /* Берем цвет предыдущей точки */
                int color=*(curdst-1);
                /* Смешиваем его с текущим цветом */
                int b=((color&0xff)+client.cur_cm[*cursrc][2])>>1;
                int g=(((color>>8)&0xff)+client.cur_cm[*cursrc][1])>>1;
                int r=(((color>>16)&0xff)+client.cur_cm[*cursrc][0])>>1;
                /* Заносим в предыдущую точку смешанный цвет */
                *(curdst-1)=b|(g<<8)|(r<<16)|(tal<<24);
                /* Сбрасываем признак пиксела повторителя */
                flg_second=0;
            }
#endif
            /* Предыдущий был одиночный пиксел */
            *curdst=colors[*cursrc];
            *curdst |= (tal<<24);
            curdst++;
            upx+=deltaw;
            if (upx>=width)
            {
                *curdst=colors[*cursrc];
                *curdst |= (tal<<24);
                curdst++;
                upx-=width;
                flg_second=1;
            }
        }
        //      memcpy (curdst,dst,sizeline);
        //      curdst+=nwidth;
        upy+=deltah;
        if (upy>=height)
        {
            memcpy (curdst,dst,sizeline);
            curdst+=nwidth;
            upy-=height;
        }
        dst=(char *)curdst;
    }
    return nheight;
#endif
#if 0   
    double x_ratio = (double)width/(double)nwidth;
    double y_ratio = (double)height/(double)nheight;
    int px, py;
    int i, j, index1, index2;//, s = 0;
    //    printf("Scale image 1!\n");
    //unsigned char red, green, blue;
    //unsigned int *i_dst = (unsigned int*)dst;
    int ixnw;
    char *it_d = dst;
    for(i = 0; i < nheight; i++){
        py = i*y_ratio;
        //ixnw = i*nwidth;
        for(j = 0; j < nwidth; j++){
            px = j*x_ratio;
            //py = floor((double)i*y_ratio);
            index1 = src[(py*width)+px];
            //index2 = (i*nwidth)+j;
            //index2 = ixnw+j;
            //index2<<=2;

            /*dst[index2] = client.cm[index1][2]>>8;
            dst[index2+1] = client.cm[index1][1]>>8;
            dst[index2+2] = client.cm[index1][0]>>8;
            dst[index2+3] = 0;*/
            *it_d++ = client.cur_cm[index1][2];//>>8;
            *it_d++ = client.cur_cm[index1][1];//>>8;
            *it_d++ = client.cur_cm[index1][0];//>>8;
            *it_d++ = 0;
            //            dst[s++] = client.cm[index][2]>>8;
            //            dst[s++] = client.cm[index][1]>>8;
            //            dst[s++] = client.cm[index][0]>>8;
        }
    }
    return nheight;
#endif
}

void RCClient_ScaleImage(int oldw, int oldh, int neww, int newh, char *src, char *dst, int n){
    int i,j;
    int h, w;
    int s = 0;
    float t, u, tmp, d1, d2, d3, d4;
    unsigned int p1, p2, p3, p4;
    unsigned char red, green, blue;
    //XColor xc;
    //Colormap colm = XDefaultColormap(client.d, XDefaultScreen(client.d));
    for(i = 0; i < newh; i++){
        tmp = (float)(i)/(float)(newh-1)*(oldh - 1);
        h = (int)floor(tmp);
        if(h < 0){
            h = 0;
        }
        else{
            if(h >= oldh - 1){
                h = oldh - 2;
            }
        }
        u = tmp - h;
        for(j = 0; j < neww; j++){
            tmp = (float)j / (float)(neww-1)*(oldw-1);
            w = (int)floor(tmp);
            if(w < 0){
                w = 0;
            }
            else{
                if(w >= oldw - 1){
                    w = oldw - 2;
                }
            }
            t = tmp-w;

            d1 = (1-t)*(1-u);
            d2 = t*(1-u);
            d3 = t * u;
            d4 = (1-t)*u;

            if(n == 4){
                p1 = p2 = p3 = p4 = 0;
                memcpy(&p1, &src[(h*oldw+w)*n], 3);
                memcpy(&p2, &src[((h*oldw+w+1)*n)], 3);
                memcpy(&p3, &src[((h+1)*oldw+w+1)*n], 3);
                memcpy(&p4, &src[((h+1)*oldw+w)*n], 3);
                blue = (unsigned char)p1 * d1 + (unsigned char)p2 * d2 + (unsigned char)p3 * d3 + (unsigned char)p4 * d4;
                green = (unsigned char)(p1 >> 8) * d1 + (unsigned char)(p2 >> 8) * d2 + (unsigned char)(p3 >> 8) * d3 + (unsigned char)(p4 >> 8) * d4;
                red = (unsigned char)(p1 >> 16) * d1 + (unsigned char)(p2 >> 16) * d2 + (unsigned char)(p3 >> 16) * d3 + (unsigned char)(p4 >> 16) * d4;

                dst[s] = blue;
                dst[s+1] = green;
                dst[s+2] = red;
                dst[s+3] = 0;
                s+=n;
            }
            else if(n == 2){
                p1 = p2 = p3 = p4 = 0;
                memcpy(&p1, &src[(h*oldw+w)*n], 2);
                memcpy(&p2, &src[((h*oldw+w+1)*n)], 2);
                memcpy(&p3, &src[((h+1)*oldw+w+1)*n], 2);
                memcpy(&p4, &src[((h+1)*oldw+w)*n], 2);

                blue = (((unsigned char)p1)&0x1f)*d1+(((unsigned char)p2)&0x1f)*d2+(((unsigned char)p3)&0x1f)*d3+(((unsigned char)p4)&0x1f)*d4;
                green = ((unsigned char)(p1>>5)&0x3f)*d1+((unsigned char)(p2>>5)&0x3f)*d2+((unsigned char)(p3>>5)&0x3f)*d3+((unsigned char)(p4>>5)&0x3f)*d4;
                red = ((unsigned char)(p1>>11)&0x1f)*d1+((unsigned char)(p2>>11)&0x1f)*d2+((unsigned char)(p3>>11)&0x1f)*d3+((unsigned char)(p4>>11)&0x1f)*d4;

                p1 = (red<<11) | ((green&0x3f)<<5) | (blue&0x1f);
                memcpy(&dst[s], &p1, 2);
                s += n;
            }
            else{
                red = client.cm[(unsigned char)src[((h*oldw+w)*n)]][0]>>8;
                green = client.cm[(unsigned char)src[((h*oldw+w)*n)]][1]>>8;
                blue = client.cm[(unsigned char)src[((h*oldw+w)*n)]][2]>>8;
                p1 = (red<<16)|(green<<8)|blue;
                red = client.cm[(unsigned char)src[((h*oldw+w+1)*n)]][0]>>8;
                green = client.cm[(unsigned char)src[((h*oldw+w+1)*n)]][1]>>8;
                blue = client.cm[(unsigned char)src[((h*oldw+w+1)*n)]][2]>>8;
                p2 = (red<<16)|(green<<8)|blue;
                red = client.cm[(unsigned char)src[(((h+1)*oldw+w+1)*n)]][0]>>8;
                green = client.cm[(unsigned char)src[(((h+1)*oldw+w+1)*n)]][1]>>8;
                blue = client.cm[(unsigned char)src[(((h+1)*oldw+w+1)*n)]][2]>>8;
                p3 = (red<<16)|(green<<8)|blue;
                red = client.cm[(unsigned char)src[(((h+1)*oldw+w)*n)]][0]>>8;
                green = client.cm[(unsigned char)src[(((h+1)*oldw+w)*n)]][1]>>8;
                blue = client.cm[(unsigned char)src[(((h+1)*oldw+w)*n)]][2]>>8;
                p4 = (red<<16)|(green<<8)|blue;

                blue = (unsigned char)p1 * d1 + (unsigned char)p2 *d2 + (unsigned char)p3 * d3 + (unsigned char)p4 * d4;
                green = (unsigned char)(p1>>8)*d1+(unsigned char)(p2>>8)*d2+(unsigned char)(p3>>8)*d3+(unsigned char)(p4>>8)*d4;
                red = (unsigned char)(p1>>16)*d1+(unsigned char)(p2>>16)*d2+(unsigned char)(p3>>16)*d3+(unsigned char)(p4>>16)*d4;

                //                xc.red = red<<8;
                //                xc.green = green<<8;
                //                xc.blue = blue<<8;
                //                xc.flags = DoRed | DoGreen | DoBlue;
                //                xc.pixel = 0;
                //                XAllocColor(client.d, colm, &xc);
                //                printf("alloc pixel - %d\n", (int)xc.pixel);
                //                dst[s] = xc.pixel;//(red<<5) | ((green&0x07)<<2) | (blue&0x03);
                //p1 = ((red&0x07)<<5) | ((green&0x07)<<2) | (blue&0x03);
                //memcpy(&dst[s], &p1, 1);
                //p1 = RCClient_GetFixedColor(red, green, blue);
                //memcpy(&dst[s], &p1, 1);
                dst[s] = blue;
                dst[s+1] = green;
                dst[s+2] = red;
                //dst[s+3] = 0;
                s+=3;
            }


            //s += n;
        }
    }
}

char *RCClient_GetVisualInfo(int *size){
    char *buf = NULL;
    int com = C_GET_VISUAL_INFO;
    *size = sizeof(int)+sizeof(VisualInfo)+sizeof(int)+sizeof(client.hname);
    buf = (char*)malloc(*size);
    memcpy(buf, size, *size);
    *size = sizeof(int);
    memcpy(&buf[*size], &com, sizeof(int));
    *size += sizeof(int);
    memcpy(&buf[*size], &client.vi, sizeof(VisualInfo));
    *size += sizeof(VisualInfo);
    memcpy(&buf[*size], client.hname, sizeof(client.hname));
    *size += sizeof(client.hname);
    return buf;
}

void *RCClient_SendThread(void *arg){
    char *buf = NULL;
    int size = 0;
    int ret = 0;
    client.th_close = 0;
    while(client.state != STATE_NONE){
        if(client.state != STATE_ACTIVE){
            buf = RCClient_GetVisualInfo(&size);
            ret = shark_send(client.sh, buf, size);
            if(ret < size){
                printf("Send to small datagram!\n");
            }
            free(buf);
            printf("Client send broadcast datagram\n");
        }
        thread_pause(TIME_WAIT);
    }
    client.th_close = 1;
    return NULL;
}

void RCClient_CompressImage(char *dst, unsigned long dst_size, char *src, unsigned long *src_size){
    void *wrkmem = malloc(LZO1X_1_MEM_COMPRESS);
    int ret = lzo1x_1_compress((unsigned char*)dst, (lzo_uint32)dst_size, (unsigned char*)src, (lzo_uintp)src_size, wrkmem);
    free(wrkmem);
    if(ret < 0) *src_size = 0;
}

void RCClient_CreateKeyPressEvent(KeySym ksym){
    KeyCode kc = XKeysymToKeycode(client.d, ksym);
    if((int)kc == 0) return;
    XTestGrabControl(client.d, True);
    XTestFakeKeyEvent(client.d, kc, True, CurrentTime);
    XTestGrabControl(client.d, False);
}

void RCClient_CreateKeyReleaseEvent(KeySym ksym){
    KeyCode kc = XKeysymToKeycode(client.d, ksym);
    if((int)kc == 0) return;
    XTestGrabControl(client.d, True);
    XTestFakeKeyEvent(client.d, kc, False, CurrentTime);
    XTestGrabControl(client.d, False);
}

void RCClient_CreateMouseMoveEvent(int x, int y){
    XTestFakeMotionEvent(client.d, DefaultScreen(client.d), x, y, CurrentTime);
    //XWarpPointer(client.d, client.root, client.root, 0, 0, 0, 0, x, y);
}

void RCClient_CreateMousePressEvent(int x, int y, int btn){
    RCClient_CreateMouseMoveEvent(x, y);
    XTestFakeButtonEvent(client.d, btn, True, CurrentTime);
}

void RCClient_CreateMouseReleaseEvent(int x, int y, int btn){
    RCClient_CreateMouseMoveEvent(x, y);
    XTestFakeButtonEvent(client.d, btn, False, CurrentTime);
}
