#ifndef PUBLIC_H
#define PUBLIC_H

#include <X11/Xlib.h>

#define SERVER_PORT 50000
#define SERVER_RECV_PORT 50003
#define CLIENT_PORT 50001
#define CLIENT_PORT_1 49000
#define CLIENT_SEND_PORT 50002
#define CLIENT_ACTIVE_PORT 50004
#define CLIENT_EVENTS_PORT 50005
#define CLIENT_IMAGE_PORT 50006

#define BROADCAST_ADDR 255
#define SMALL_SCREEN_WIDTH 300
#define SMALL_SCREEN_HEIGHT 200

#define RECEIVE_OK 0
#define RECEIVE_FAILED 1

#define TIME_WAIT 4000

#define MAX_IMAGE_SIZE 800000

typedef enum ServerClientStates{
    STATE_NONE,
    STATE_ACTIVE,
    STATE_PASSIVE
}ServerClientStates;

typedef enum Commands{
    C_GET_SCREEN,
    C_GET_VISUAL_INFO,
    C_SET_EVENT,
    C_GET_SMALL_SCREEN,
    C_SET_ACTIVE_STATE,
    C_SET_PASSIVE_STATE,
    C_SET_VISUAL_INFO,
    C_GET_COLORMAP,
    C_SET_COLORMAP,
    C_SET_MOUSE_COORD,
    C_GET_FULL_IMAGE
}Commands;

typedef enum MouseEvents{
    MOUSE_PRESS,
    MOUSE_RELEASE,
    MOUSE_CLICK,
    MOUSE_DOUBLE_CLICK,
    MOUSE_MOVE
}MouseEvents;

typedef enum KeyEvents{
    KEY_PRESS,
    KEY_RELEASE
}KeyEvents;

typedef enum EventTypes{
    EVENT_TYPE_MOUSE,
    EVENT_TYPE_KEY
}EventTypes;

typedef struct MouseEvent{
    //int x, y;
    float x, y;
    int button;
    MouseEvents event;
}MouseEvent;

typedef struct KeyEvent{
    unsigned int keySym;
    KeyEvents event;
}KeyEvent;

typedef struct Events{
    int type;
    void *event;
}Events;

typedef struct VisualInfo{
    int screen_width;
    int screen_height;
    int depth;
    int n;
}VisualInfo;

typedef struct ImageInfo{
    int x, y;
    int width, height;
    int size;
    int depth;
    char rle;
    int cur_height;
}ImageInfo;

typedef struct MvPanel_Info{
    void *panel;
    int x, y;
}MvPanel_Info;

#endif // PUBLIC_H

