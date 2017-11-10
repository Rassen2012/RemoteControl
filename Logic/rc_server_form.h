#ifndef RC_SERVER_FORM_H
#define RC_SERVER_FORM_H

#include "pthread.h"
#include "Widgets/form.h"
#include "shark.h"
#include "eventsqueue.h"

#define SF_WIDTH 700
#define SF_HEIGHT 500


typedef struct RCServerForm{
    Form *f;
    shark *sh;
    pthread_t m_th;
    pthread_t im_th;
    pthread_t pack_th;
    pthread_mutex_t mutex;
    //pthread_mutex_t p_mutex;
    EventQueue *qu;
    int client_addr;
    int client_port;
    int server_port;
    int client_depth;
    int client_width;
    int client_height;
    char *client_saddr;
    int tmp_width, tmp_height;
    unsigned short pal[256][3];
    unsigned short serv_pal[256][3];
    /*******************************/
    unsigned char serv_pal_s[256][3];
    unsigned char ap_pal[256];
    /*******************************/
    /*******************************/
    unsigned int *big_pal;
    /*******************************/
    unsigned long move_cur_time;
    /*******************************/
    int image_x;
    int image_y;
    /*******************************/
    unsigned long resize_time;
    char resize_flag;
    /*******************************/
    char active;
    int client_mouse_x;
    int client_mouse_y;
    Cursor cursor;
    char *title;
	char chName[100];
    char titleVisible;
    int curp_x, curp_y;
    unsigned int curp_width, curp_height;
    char disconnected; // ?
    char titlePressed;
    char crHover;
    unsigned long sw_time;

    char isFullWindow;
    int fullx, fully, fullwidth, fullheight;

    int x, y;

    void (*Close)(int);

    char *imageData;
    int image_width, image_height;
    int n;
}RCServerForm;

RCServerForm *RCServerForm_newRCServerForm(int client_addr, int client_port, int server_port, int client_depth, int client_width, int client_height, unsigned short pal[][3],
                                unsigned short serv_pal[][3], char *hostIp, char *chname, char *s_addr);
void RCServerForm_Dispose(RCServerForm *sf);
void RCServerForm_Start(RCServerForm *sf, int x, int y, int width, int height);
void RCServerForm_Stop(RCServerForm *sf);
void RCServerForm_Raise(RCServerForm *sf);
char *RCServerForm_GetImageData(RCServerForm *sf, int *width, int *height, int *n);

#endif // RC_SERVER_FORM_H

