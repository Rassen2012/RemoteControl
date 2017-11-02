#include <stdio.h>
#ifdef linux
#include <malloc.h>
#include <X11/xpm.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
//#include <X11/xpm.h>
#include "public.h"
#include "../Widgets/form.h"
#include "../Widgets/panel.h"
#include "../Widgets/listview.h"
#include "../Widgets/label.h"
#include "../Widgets/textline.h"
#if defined (OC2K1x)
#include "general.h"
#else
#include "system.h"
#endif
#include "shark.h"
#include "minilzo/minilzo.h"
#include "rc_server.h"
#include "eventsqueue.h"
#include "rc_server_form.h"
#include "rc_client.h"

#define PANEL_WIDTH 256//200
#define PANEL_HEIGHT 192//120
#define FORM_WIDTH PANEL_WIDTH*3+20*4//680
#define FORM_HEIGHT PANEL_HEIGHT*3+20*4//480
#define TITLE_HEIGHT 20
#define BIG_PALETE_SIZE 65536

typedef struct MRectangle{
    int x, y;
    int width, height;
}MRectangle;

typedef struct ImageParam{
    char *im;
    int addr;
    int width, height, n;
}ImageParam;

typedef struct ClientInfo{
    VisualInfo vi;
    int addr;
    unsigned long lastActive;
    RCServerForm *sf;
    char active;
    unsigned short cm[256][3];
    unsigned char a_cm[256][3];
    unsigned char ap_cm[256];
    char *s_addr;
    char *chname;
    char preview;
    int sf_x, sf_y;
    int sf_width, sf_height;
}ClientInfo;

typedef struct RCServer{
    Form *f;
    S_ListView *lv;
    Panel *p;
    TextLine *tl;

    ClientInfo **ci;
    ServerClientStates state;
    shark *sh;
    int client_count;
    pthread_t thCheck;
    pthread_t thEvent;
    pthread_t thPaint;
    pthread_t *thCheckClient;
    pthread_mutex_t mutex;
    pthread_mutex_t paint_mutex;
    VisualInfo vi;
    int next_port;
    unsigned short cm[256][3];
    unsigned char s_pal[256][3];
    unsigned int *big_pal;

    EventQueue *qu;
    unsigned char localAddr;
}Server;

Server server = {0};

void RCServer_PanelMousePress(Panel *p, MouseEventArgs *ev);

void RCServer_Receive(int addr, char *buf, int size);
void RCServer_AddClient(int addr, VisualInfo *vi, char *s_addr, char *chname);
void RCServer_RemoveClient(int id);
void RCServer_PanelCreateImage(void *arg);
void RCServer_CreateImage(int id, unsigned char *buf);
void RCServer_SendGetSmallImage(int id);
int RCServer_SendGetScreen(shark *sh, int width, int height, int id);
void *RCServer_CheckClientsThread(void *arg);
void RCServer_GetActiveClient(shark *sh);
void RCServer_CloseActiveClient(int addr);

int GetAppPalete(unsigned char r, unsigned char g, unsigned char b);
void RCServer_MakeBigPalette();

void *FormPaint_Thread(void *arg);
void *RCServer_CheckClientThread(void *arg);

void RCServer_ButtonHover(int addr, int x);
void RCServer_ButtonLeave(int addr);
void RCServer_ButtonClick(int addr);
void RCServer_HideButtonClick();

void RCServer_GetVisualInfo(){
    XWindowAttributes attr;
    Display *d = XOpenDisplay(NULL);
    Window root = XRootWindow(d, XDefaultScreen(d));
    XGetWindowAttributes(d, root, &attr);
    server.vi.screen_width = attr.width;
    server.vi.screen_height = attr.height;
    server.vi.depth = attr.depth;

    Color_SetDepth(DefaultDepth(d, DefaultScreen(d)), d);
    switch(server.vi.depth){
    case 8: server.vi.n = 8; break;
    case 16: server.vi.n = 16; break;
    case 24: server.vi.n = 32; break;
    }

    if(server.vi.depth != 8) return;
    unsigned short cm[256][3];// = {0};
    XColor col;
    Colormap colm = XDefaultColormap(d, XDefaultScreen(d));
    int i;
    for(i = 0; i < 256; i++){
        memset(&col, 0, sizeof(XColor));
        col.pixel = i;
        XQueryColor(d, colm, &col);
        cm[i][0] = col.red;
        cm[i][1] = col.green;
        cm[i][2] = col.blue;
    }
    memcpy(server.cm, cm, sizeof(cm));
    for(i = 0; i < 256; i++){
        server.s_pal[i][0] = server.cm[i][0]>>8;
        server.s_pal[i][1] = server.cm[i][1]>>8;
        server.s_pal[i][2] = server.cm[i][2]>>8;
    }
    XCloseDisplay(d);
}

char *RCServer_RedepthImage16x24(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*size*2);
    int i, j;
    unsigned int c;
    size_t step = sizeof(unsigned int);
    for(i = 0, j = 0; i < size; i+=2, j+=4){
        memcpy(&c, &src[i], 2);
        c = ((c<<3) & 0xf8) | ((c>>2)&0x7) | ((c<<5) & 0xfc00) | ((c>>1) & 0x300) | ((c<<8) & 0xf80000) | ((c<<3) & 0x70000);
        memcpy(&dst[j], &c, step);
    }
    return dst;
}

int RCServer_GetFixedColor(unsigned int c){
    int i, index, val1, val2, val3;
    index = 0;
    val3 = -1;
    val2 = 0;
    val1 = c;
    for(i = 0; i < 256; i++){
        val2 = ((unsigned char)(server.cm[i][0]>>8) << 16) | ((unsigned char)(server.cm[i][1]>>8) << 8) | (unsigned char)(server.cm[i][2]>>8);
        if(abs(val1 - val2) < val3 || val3 < 0){
            index = i;
            val3 = abs(val1 - val2);
            if(val3 == 0) break;
        }
    }
    return index;
}

int RCServer_GetFixedColor16(unsigned int c){
    int i, index, val1, val2, val3;
    index = 0;
    val3 = -1;
    val2 = 0;
    val1 = c;
    for(i = 0; i < 256; i++){
        val2 = ((unsigned char)(server.cm[i][0]>>8) << 11) | (((unsigned char)(server.cm[i][1]>>8)&0x3f) << 5) | ((unsigned char)(server.cm[i][2]>>8)&0x1f);
        if(abs(val1 - val2) < val3 || val3 < 0){
            index = i;
            val3 = abs(val1 - val2);
            if(val3 == 0) break;
        }
    }
    return index;
}

char *RCServer_RedepthImage24x8(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*size/4);
    int i, j, val;
    unsigned int *it = (unsigned int*)src;
    for(i = 0, j = 0; i < size; i+=4, it++){
        val = (((*it>>19)&0x1f)<<11) | (((*it>>10)&0x3f)<<5) | ((*it>>3)&0x1f);
        dst[j++] = server.big_pal[val];
    }
    return dst;
}

char *RCServer_RedepthImage16x8(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*size/2);
    int i, j;
    unsigned short *dsrc = (unsigned short*)src;
    for(i = 0, j = 0; i < size; i+=2){
        dst[j++] = server.big_pal[*dsrc++];
    }
    return dst;
}

char *RCServer_RedepthImage8x24(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*size*4);
    memcpy(dst, src, size);
    return dst;
}

char *RCServer_RedepthImage8x8(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*size/4);
    int i, j, val;
    unsigned int *it = (unsigned int*)src;
    for(i = 0, j = 0; i < size; i+=4, it++){
        val = (((*it>>19)&0x1f)<<11) | (((*it>>10)&0x3f)<<5) | ((*it>>3)&0x1f);
        dst[j++] = server.big_pal[val];
    }
    return dst;
}

char *RCServer_RedepthImage8x16(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*(size/2));
    int i, j;
    unsigned int c;
    unsigned short *dstx = (unsigned short*)dst;
    unsigned char r, g, b;
    size_t step = 2;
    for(i = 0, j = 0; i < size; i+=4, j+=step){
        r = src[i+2];//(unsigned char)(ci->cm[(unsigned char)src[i]][0]>>8);
        g = src[i+1];//(unsigned char)(ci->cm[(unsigned char)src[i]][1]>>8);
        b = src[i];//(unsigned char)(ci->cm[(unsigned char)src[i]][2]>>8);
        c = (r<<16)|(g<<8)|b;
        c = Color_16From24(c);
        //memcpy(&dst[j], &c, step);
        *dstx++ = (unsigned short)c;
    }
    return dst;
}

char *RCServer_RedepthImage24x16(char *src, int size){
    char *dst = (char*)malloc(sizeof(char)*(size/2));
    int i, j;
    unsigned int c;
    for(i = 0, j = 0; i < size; i+=4, j+=2){
        memcpy(&c, &src[i], sizeof(unsigned int));
        c = Color_16From24(c);
        memcpy(&dst[j], &c, sizeof(unsigned int));
    }
    return dst;
}

void RCServer_DecompressImage(char *src, unsigned long src_size, char *dst, unsigned long *dst_size){
    int ret = lzo1x_decompress((unsigned char*)src,  (lzo_uint32)src_size, (unsigned char*)dst, (lzo_uintp)dst_size, 0);
    if(ret < 0) *dst_size = 0;
}

void RCServer_SetPanelImage(void *arg){
    char *data = (char*)arg;
    XImage *im = XCreateImage(server.p->widget->display, DefaultVisual(server.p->widget->display, DefaultScreen(server.p->widget->display)), server.vi.depth, ZPixmap, 0,
                              data, server.p->widget->width, server.p->widget->height, server.vi.n, 0);
    if(im) Panel_SetBackgroundImage(server.p, im);
}

int RCServer_SendGetScreen(shark *sh, int width, int height, int addr){
    int size = sizeof(int)*2+sizeof(ImageInfo);
    int com = C_GET_SCREEN;
    unsigned long t, f;//, ttime;
    int ret;
    int result = -1;
    char sndBuf[SHARK_BUFFSIZE] = {0};
    char *recvBuf;
    char *imageBuf;
    char *tmp;
    ImageInfo imInfo;
    imInfo.width = width;
    imInfo.height = height;
    //ttime = time_get();
    memcpy(sndBuf, &size, sizeof(int));
    memcpy(&sndBuf[sizeof(int)], &com, sizeof(int));
    memcpy(&sndBuf[sizeof(int)*2], &imInfo, sizeof(ImageInfo));
    sh = shark_init(CLIENT_PORT, SERVER_RECV_PORT+addr, addr, NULL);
    ret = shark_send(sh, sndBuf, size);
    //	printf("Begin recv screen!\n");
    if(ret < size){
        printf("Receive screen failed 1\n");
        shark_close(sh);
        return result;
    }
    //int tmpAddr = sh->addr.sin_addr.s_addr>>24;
    shark_close(sh);
    sh = shark_init(CLIENT_PORT_1+addr, SERVER_RECV_PORT+addr, addr, NULL);
    //printf("Before read - %d\n", time_get() - ttime);
    //ttime = time_get();
    recvBuf = shark_read(sh, 0, &size);
   // printf("read time - %d\n", time_get() - ttime);
    if(size < (int)(sizeof(int)+sizeof(ImageInfo))){
        printf("Receive screen failed 2\n");
        shark_close(sh);
        return result;
    }
    else{
        memcpy(&com, recvBuf, sizeof(int));
        if(com == C_GET_SCREEN){
            memcpy(&imInfo, &recvBuf[sizeof(int)], sizeof(ImageInfo));
     //       printf("Recv image info - %d\n", time_get() - ttime);
       //     ttime = time_get();
            com = RECEIVE_OK;
            memset(sndBuf, 0, sizeof(sndBuf));
            memcpy(sndBuf, &com, sizeof(int));
            ret = shark_send(sh, sndBuf, sizeof(int));
            if(ret < (int)sizeof(int)){
                printf("Receive screen failed 3\n");
                shark_close(sh);
                return result;
            }
            imageBuf = (char*)malloc(imInfo.size);
            size = shark_recv_all(sh, imageBuf, imInfo.size);
            if(size < imInfo.size){
                free(imageBuf);
                printf("Receive screen failed 4\n");
                shark_close(sh);
                return result;
            }
         //   printf("Recv all time - %d\n", time_get() -ttime);
          //  ttime = time_get();
            if(imInfo.depth == 24){/* n = 32;*/ t = 4; }
            else if(imInfo.depth == 16){ /*n = 16;*/ t = 2; }
            else{ /*n = 8;*/ t = 4; }
            t = imInfo.width*imInfo.height*t;
            //			t *= 2;
            tmp = (char*)malloc(t);
            f = imInfo.size;
            //			printf("Begin decompress!\cn");
            RCServer_DecompressImage(imageBuf, f, tmp, &t);
            //			printf("End decompress!\n");
            if(t == 0){
                free(tmp);
                free(imageBuf);
                shark_close(sh);
                return result;
            }
            free(imageBuf);
            shark_close(sh);
            imageBuf = tmp;
            imInfo.size = t;
            if(imInfo.depth != server.vi.depth){
                if(imInfo.depth == 16 && server.vi.depth == 24){
                    //tmp = RCServer_RedepthImage16x24(imageBuf, imInfo.size);
                    tmp = Graphics_RedepthImage(imageBuf, imInfo.size, 16, 24, NULL, NULL);
                    free(imageBuf);
                    imageBuf = tmp;
                    //n = 32;
                }
                else if(imInfo.depth == 24 && server.vi.depth == 16){
                    //tmp = RCServer_RedepthImage24x16(imageBuf, imInfo.size);
                    tmp = Graphics_RedepthImage(imageBuf, imInfo.size, 24, 16, NULL, NULL);
                    free(imageBuf);
                    imageBuf = tmp;
                    //n = 16;
                }
                else if(imInfo.depth == 24 && server.vi.depth == 8){
                    tmp = RCServer_RedepthImage24x8(imageBuf, imInfo.size);
                    //tmp = Graphics_RedepthImage(imageBuf, imInfo.size, 24, 8, NULL, server.big_pal);
                    free(imageBuf);
                    imageBuf = tmp;
                    //n = 8;
                }
                else if(imInfo.depth == 8 && server.vi.depth == 24){
                    tmp = RCServer_RedepthImage8x24(imageBuf, imInfo.size);
                    free(imageBuf);
                    imageBuf = tmp;
                    //n = 32;
                }
                else if(imInfo.depth == 8 && server.vi.depth == 16){
                    tmp = RCServer_RedepthImage8x16(imageBuf, imInfo.size);
                    free(imageBuf);
                    imageBuf = tmp;
                    //n = 16;
                }
                else if(imInfo.depth == 16 && server.vi.depth == 8){
                    tmp = RCServer_RedepthImage16x8(imageBuf, imInfo.size);
                    //tmp = Graphics_RedepthImage(imageBuf, imInfo.size, 16, 8, NULL, server.big_pal);
                    free(imageBuf);
                    imageBuf = tmp;
                    //n = 8;
                }
            }
            else if(imInfo.depth == 8 && server.vi.depth == 8){
                tmp = RCServer_RedepthImage8x8(imageBuf, imInfo.size);
                //tmp = Graphics_RedepthImage(imageBuf, imInfo.size, 8, 8, NULL, server.big_pal);
                free(imageBuf);
                imageBuf = tmp;
            }
            Event ev;
            ev.func = RCServer_SetPanelImage;
            ev.arg = (void*)imageBuf;
            Form_AddEvent(server.f, ev);
        }
    }
    //	printf("Recv screen end!\n");
    return 0;
}

void RCServer_Close(){
    int i;
    Event ev;
    ev.func = NULL;
    ev.type = EV_CLOSE_WINDOW;
    server.state = STATE_NONE;
    for(i = 0; i < server.client_count; i++){
        if(server.ci[i]->active){
            Form_AddEvent(server.ci[i]->sf->f, ev);
        }
        while(server.ci[i]->active) thread_pause(1);
    }
    pthread_join(server.thCheck, NULL);
    shark_close(server.sh);
}

void RCServer_MakeBigPalette(){
    int i;
    server.big_pal = (unsigned int*)malloc(sizeof(unsigned int)*BIG_PALETE_SIZE);
    for(i = 0; i < BIG_PALETE_SIZE; i++){
        server.big_pal[i] = GetAppPalete((i>>11)<<3,((i>>5)&0x3f)<<2, (i&0x1f)<<3);
    }
}

void RCServer_Start(){
    if(server.state != STATE_NONE){
        printf("Один экземпляр сервера уже запущен!\n");
        return;
    }
    server.localAddr = shark_getLocalAddr();
    server.ci = NULL;
    server.state = STATE_PASSIVE;
    server.sh = shark_init(CLIENT_PORT, SERVER_PORT, BROADCAST_ADDR, RCServer_Receive);
    server.client_count = 0;
    server.qu = NULL;
    server.next_port = 50010;
    pthread_mutex_init(&server.mutex, NULL);
    pthread_mutex_init(&server.paint_mutex, NULL);
    RCServer_GetVisualInfo();
    if(server.vi.depth == 8){
        RCServer_MakeBigPalette();
    }

    server.f = Form_newForm(0, 0, 700, 500, MAIN_FORM, NULL);
    Form_SetTitle(server.f, "Remote control - server!");
    Form_SetSizePolicy(server.f, server.f->widget->width, server.f->widget->height, server.f->widget->width, server.f->widget->height);
    server.tl = TextLine_newTextLine(server.f->widget, 5, 5, 250, 22, "Введите ip...", 10);
    server.lv = ListView_newListView(server.f->widget, 5, 32, 250, 460);
    server.p = Panel_newPanel(260, 5, 435, 250, server.f->widget);

    Widget_SetTabIndex(server.tl->widget, 0);
    Widget_SetTabIndex(server.lv->widget, 1);
    Widget_SetTabIndex(server.p->widget, 2);
    Widget_Tab(server.tl->widget);
    Form_Show(server.f);
}

int GetAppPalete(unsigned char r, unsigned char g, unsigned char b){
    int i, res, index = 0, min = 1000000;
    for(i = 0; i < 256; i++){
        res = 30 * (server.s_pal[i][0] - r)*(server.s_pal[i][0]-r) + 59*(server.s_pal[i][1]-g)*(server.s_pal[i][1]-g) + 11*(server.s_pal[i][2] - b)*(server.s_pal[i][2] - b);
        if(res < min){
            index = i;
            min = res;
        }
    }
    return index;
}

void RCServer_SendGetColormap(ClientInfo *ci){
    char buf[SHARK_BUFFSIZE] = {0};
    char *tmp;
    while(ci->addr == 0) thread_pause(10);
    shark *sh = shark_init(CLIENT_PORT, SERVER_RECV_PORT, ci->addr, NULL);
    int com = C_GET_COLORMAP;
    int size = sizeof(int)*2;
    memcpy(buf, &size, sizeof(int));
    memcpy(&buf[sizeof(int)], &com, sizeof(int));
    shark_send(sh, buf, size);
    tmp = shark_read(sh, 0, &size);
    if(size > 0){
        memcpy(ci->cm, &tmp[sizeof(int)*2], sizeof(ci->cm));
    }
    shark_close(sh);
    int i;
    for(i = 0; i < 256; i++){
        ci->a_cm[i][0] = ci->cm[i][0]>>8;
        ci->a_cm[i][1] = ci->cm[i][1]>>8;
        ci->a_cm[i][2] = ci->cm[i][2]>>8;
    }
    if(memcmp(server.s_pal, ci->a_cm, sizeof(server.s_pal)) != 0){
        for(i = 0; i < 256; i++){
            ci->ap_cm[i] = GetAppPalete(ci->a_cm[i][0], ci->a_cm[i][1], ci->a_cm[i][2]);
        }
    }
}

void RCServer_Receive(int addr, char *buf, int size){
    int buf_size = 0;
    int com;
    int i;
    VisualInfo vi = {0};
    if(addr == server.localAddr) return;
    //printf("Addr = %d, local addr - %d\n", addr, server.localAddr);
    if(size < (int)sizeof(int)){
        printf("Server receive to small datagram\n");
        return;
    }
    memcpy(&buf_size, buf, sizeof(int));
    if(buf_size < size) return;
    if(buf_size < (int)sizeof(int)*2) return;
    memcpy(&com, &buf[sizeof(int)], sizeof(int));
    for(i = 0; i < server.client_count; i++){
        if(server.ci[i]->addr == addr){
            if(com == C_GET_COLORMAP){
                memcpy(&server.ci[i]->cm, &buf[sizeof(int)*2], sizeof(server.ci[i]->cm));
            }
            server.ci[i]->lastActive = time_get();
            return;
        }
    }
    if(com != C_GET_VISUAL_INFO) {
        return;
    }
    memcpy(&vi, &buf[sizeof(int)*2], sizeof(VisualInfo));
    char hname[100] = {0};
    memcpy(hname, &buf[sizeof(int)*2 + sizeof(VisualInfo)], sizeof(hname));
    RCServer_AddClient(addr, &vi, server.sh->cl_addr, hname);
//    if(vi.depth == 8) {
//        pthread_mutex_lock(&server.mutex);
//        RCServer_SendGetColormap(&server.ci[server.client_count-1]);
//        pthread_mutex_unlock(&server.mutex);
//    }
}

void RCServer_AddClient(int addr, VisualInfo *vi, char *s_addr, char *chname){
    ClientInfo *ci = (ClientInfo*)malloc(sizeof(ClientInfo));
    ci->addr = addr;
    ci->active = 0;
    ci->s_addr = strdup(s_addr);
    ci->sf_x = ci->sf_y = ci->sf_width = ci->sf_height = 0;
    ci->preview = 0;
    memcpy(&ci->vi, vi, sizeof(VisualInfo));
    ci->chname = strdup(chname);

    ci->lastActive = time_get();
    server.ci = (ClientInfo**)realloc(server.ci, sizeof(ClientInfo*)*(server.client_count+1));
    server.ci[server.client_count] = ci;
    server.client_count++;

    pthread_t th;
#if defined (OC2K1x)
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = 90;
    pthread_attr_setschedparam(&attr, &param);
    //pthread_create(&th, &attr, RCServer_CheckClientThread, (void*)&server.ci[server.client_count-1]);
    pthread_create(&th, &attr, RCServer_CheckClientThread, (void*)ci);
#else
    //pthread_create(&th, NULL, RCServer_CheckClientThread, (void*)&server.ci[server.client_count-1]);
    pthread_create(&th, NULL, RCServer_CheckClientThread, (void*)ci);
#endif
}

void RCServer_RemoveClient(int id){
    if(server.state == STATE_ACTIVE) return;
    int i;
    for(i = 0; i < server.client_count; i++){
        if(server.ci[i]->addr == id){
            id = i;
            break;
        }
    }
    pthread_mutex_lock(&server.paint_mutex);
    ClientInfo **tmpCi;
    if(server.client_count > 1){
        tmpCi = (ClientInfo**)malloc(sizeof(ClientInfo*)*server.client_count);
        memcpy(tmpCi, server.ci, sizeof(ClientInfo*)*server.client_count);
        server.ci = (ClientInfo**)malloc(sizeof(ClientInfo*)*(server.client_count-1));
        memcpy(server.ci, tmpCi, sizeof(ClientInfo*)*id);
        memcpy(&server.ci[id], &tmpCi[id+1], sizeof(ClientInfo*)*(server.client_count - (id+1)));
        free(tmpCi);
    }
    else{
        free(server.ci);
        server.ci = NULL;
    }
    --server.client_count;
    pthread_mutex_unlock(&server.paint_mutex);
}

void RCServer_Stop(){
    if(server.state == STATE_NONE || server.f == NULL) return;
    Event ev;
    ev.func = NULL;
    ev.type = EV_CLOSE_WINDOW;
    Form_AddEvent(server.f, ev);
}

void RCServer_CalcWindowPos(int *x, int *y, int width, int height){

    int p = server.client_count%4;

    MRectangle rects[4] = {};
    if(p == 0) {
        *x = 0;
        *y = 0;
        return;
    }
    int i;
    //First rect
    rects[0].width = server.vi.screen_width>>1;
    rects[0].height = server.vi.screen_height>>1;
    for(i = 0; i < p; i++){
        int index = server.client_count - (p-i);
        MRectangle mrects[2] = {};
        if(!server.ci[index]->active) continue;
        S_Widget *twid = server.ci[index]->sf->f->widget;
        if(twid->x > rects[0].width || twid->y > rects[0].height) continue;
        if(twid->x > 0){
            mrects[0].width = twid->x;
            mrects[0].height = rects[1].height;
        }
        else{
            mrects[0].width = rects[0].width - (twid->x+twid->width);
            mrects[0].height = rects[1].height;
        }
        if(twid->y > 0){
            mrects[1].width = rects[0].width;
            mrects[1].height = twid->y;
        }
        else{
            mrects[1].width = rects[0].width;
            mrects[1].height = rects[0].height - (twid->y+twid->height);
        }

        mrects[0].width = rects[0].width;
        mrects[0].height = twid->y;
        mrects[1].width = twid->x;
        mrects[1].height = rects[0].height;
        int s1 = mrects[0].width*mrects[0].height;
        int s2 = mrects[1].width*mrects[1].height;
        int s3 = rects[0].width*rects[0].height;
        if(s1 > s2){
            if(s1 < s3){
                memcpy(&rects[0], &mrects[0], sizeof(MRectangle));
            }
        }
        else{
            if(s2 < s3){
                memcpy(&rects[0], &mrects[1], sizeof(MRectangle));
            }
        }
    }
    //Second rects
    rects[1].width = server.vi.screen_width>>1;
    rects[1].height = server.vi.screen_height>>1;
    rects[1].x = 0;
    rects[1].y = rects[1].height;
    for(i = 0; i < p; i++){
        int index = server.client_count - (p - i);
        MRectangle mrects[2] = {};
        if(!server.ci[index]->active) continue;
        S_Widget *twid = server.ci[index]->sf->f->widget;
        if(twid->x > rects[1].width || twid->y < rects[1].y) continue;
        if(twid->x > 0){
            mrects[0].width = twid->x;
            mrects[0].height = rects[1].height;
        }
        else{
            mrects[0].width = rects[1].width - (twid->x + twid->width);
            mrects[0].height = rects[1].height;
        }
        if(twid->y + twid->height < server.vi.screen_height){
            mrects[1].width = rects[1].width;
            mrects[1].height = server.vi.screen_height - (twid->y+twid->height);
        }
        else{
            mrects[1].width = rects[1].width;
            mrects[1].height = rects[1].height - twid->y;
        }
        int s1 = mrects[0].width*mrects[0].height;
        int s2 = mrects[1].width*mrects[1].height;
        int s3 = rects[1].width*rects[1].height;
        if(s1 > s2){
            if(s1 < s3){
                memcpy(&rects[1], &mrects[0], sizeof(MRectangle));
            }
        }
        else{
            if(s2 < s3){
                memcpy(&rects[1], &mrects[1], sizeof(MRectangle));
            }
        }
    }

    rects[2].width = server.vi.screen_width>>1;
    rects[2].height = server.vi.screen_height>>1;
    rects[2].x = rects[2].width;
    rects[2].y = 0;
    for(i = 0; i < p; i++){
        int index = server.client_count - (p - i);
        MRectangle mrects[2] = {};
        if(!server.ci[index]->active) continue;
        S_Widget *twid = server.ci[index]->sf->f->widget;
        if(twid->x + twid->width < rects[2].width || twid->y > rects[2].height) continue;
        if(twid->x + twid->width < server.vi.screen_width){
            mrects[0].width = server.vi.screen_width - (twid->x+twid->width);
            mrects[0].height = rects[2].height;
        }
        else{
            mrects[0].width = rects[2].width - twid->x;
            mrects[0].height = rects[2].height;
        }
        if(twid->y > 0){
            mrects[1].width = rects[2].width;
            mrects[1].height = twid->y;
        }
        else{
            mrects[1].width = rects[2].width;
            mrects[1].height = rects[2].height - (twid->y+twid->height);
        }

        int s1 = mrects[0].width*mrects[0].height;
        int s2 = mrects[1].width*mrects[1].height;
        int s3 = rects[2].width*rects[2].height;
        if(s1 > s2){
            if(s1 < s3){
                memcpy(&rects[2], &mrects[0], sizeof(MRectangle));
            }
        }
        else{
            if(s2 < s3){
                memcpy(&rects[2], &mrects[1], sizeof(MRectangle));
            }
        }
    }

    rects[3].width = server.vi.screen_width>>1;
    rects[3].height = server.vi.screen_height>>1;
    rects[3].x = rects[3].width;
    rects[3].y = rects[3].height;
    for(i = 0; i < p; i++){
        int index = server.client_count - (p - i);
        MRectangle mrects[2] = {};
        if(!server.ci[index]->active) continue;
        S_Widget *twid = server.ci[index]->sf->f->widget;
        if(twid->x + twid->width < rects[2].width || twid->y + twid->height < rects[2].height) continue;
        if(twid->x + twid->width < server.vi.screen_width){
            mrects[0].width = server.vi.screen_width - (twid->x + twid->width);
            mrects[0].height = rects[3].height;
        }
        else{
            mrects[0].width = rects[3].width - (server.vi.screen_width - twid->x);
            mrects[0].height = rects[3].height;
        }
        mrects[1].width = rects[3].width;
        mrects[2].height = server.vi.screen_height - (twid->y + twid->height);
        int s1 = mrects[0].width*mrects[0].height;
        int s2 = mrects[1].width*mrects[1].height;
        int s3 = rects[3].width*rects[3].height;
        if(s1 > s2){
            if(s1 < s3){
                memcpy(&rects[3], &mrects[0], sizeof(MRectangle));
            }
        }
        else{
            if(s2 < s3){
                memcpy(&rects[3], &mrects[1], sizeof(MRectangle));
            }
        }
    }

    int ss[4] = {0};
    for(i = 0; i < 4; i++){
        ss[i] = rects[i].width*rects[i].height;
    }
    int res  = ss[0];
    int ind = 0;
    for(i = 1; i < 4; i++){
        if(ss[i]>res) {
            res = ss[i];
            ind = i;
        }
    }

    switch(ind){
    case 1:
        *x = 0;
        *y = server.vi.screen_height - height;
        break;
    case 2:
        *x = server.vi.screen_width - width;
        *y = 0;
        break;
    case 3:
        *x = server.vi.screen_width - width;
        *y = server.vi.screen_height - height;
        break;
    case 0:
    default:
        *x = *y = 0;
        break;
    }

}

void RCServer_PanelMousePress(Panel *p, MouseEventArgs *ev){

}

void RCServer_GetActiveClient(shark *sh){
    char buf[SHARK_BUFFSIZE] = {0};
    int size = sizeof(int)*3;
    int com = C_SET_ACTIVE_STATE;
    int port = server.next_port;//50010;//sh->portsnd;
    memcpy(buf, &size, sizeof(int));
    memcpy(&buf[sizeof(int)], &com, sizeof(int));
    memcpy(&buf[sizeof(int)*2], &port, sizeof(int));
    shark_send(sh, buf, size);
}

void RCServer_SetPassiveClient(shark *sh){
    char buf[SHARK_BUFFSIZE] = {0};
    int size = sizeof(int)*2;
    int com = C_SET_PASSIVE_STATE;
    memcpy(buf, &size, sizeof(int));
    memcpy(&buf[sizeof(int)], &com, sizeof(int));
    shark_send(sh, buf, size);
}

void RCServer_CloseActiveClient(int addr){
    int i;
    for(i = 0; i < server.client_count; i++){
        if(server.ci[i]->addr == addr){
            server.ci[i]->sf_x = server.ci[i]->sf->f->widget->x;
            server.ci[i]->sf_y = server.ci[i]->sf->f->widget->y;
            server.ci[i]->sf_width = server.ci[i]->sf->f->widget->width;
            server.ci[i]->sf_height = server.ci[i]->sf->f->widget->height;
            server.ci[i]->lastActive = time_get();
            server.ci[i]->active = 0;
            //TopForm_ClientClose(server.ci[i]->addr);
            break;
        }
    }
}

void *RCServer_CheckClientThread(void *arg){
    ClientInfo *ci = (ClientInfo*)arg;
    int a = 0;
    while(server.state != STATE_NONE){
            if(ci->preview){
                if(RCServer_SendGetScreen(NULL, PANEL_WIDTH, PANEL_HEIGHT, ci->addr) < 0){
                if(++a > 4){
                    pthread_mutex_lock(&server.mutex);
                    RCServer_RemoveClient(ci->addr);
                    pthread_mutex_unlock(&server.mutex);
                    break;
                }
                }
                else a = 0;
            }
            else if(time_get() - ci->lastActive > (TIME_WAIT<<1) && !ci->active){
                pthread_mutex_lock(&server.mutex);
                RCServer_RemoveClient(ci->addr);
                pthread_mutex_unlock(&server.mutex);
                break;
            }
        thread_pause(400);
    }
    return NULL;
}
