#ifndef MAINFORM
#define MAINFORM

#include "form.h"
#include "panel.h"
#include "listview.h"
#include "../Logic/rc_server.h"

typedef struct MainForm{
    Form *f;
    S_ListView *lv;
    Panel *p;

    RCServer *server;
}MainForm;

#endif // MAINFORM

