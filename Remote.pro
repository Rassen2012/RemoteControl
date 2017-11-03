TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lX11 -lXpm -lpthread -lXtst
DEFINES += SELF_COMPILE LINUX_MINT

SOURCES += \
    Logic/minilzo/minilzo.c \
    Logic/rc_client.c \
    Logic/rc_server.c \
    Logic/rc_server_form.c \
    Logic/shark.c \
    Logic/system.c \
    Widgets/button.c \
    Widgets/color.c \
    Widgets/events.c \
    Widgets/eventstack.c \
    Widgets/form.c \
    Widgets/graphics.c \
    Widgets/label.c \
    Widgets/panel.c \
    Widgets/widget.c \
    eventsqueue.c \
    Logic/main.c \
    Logic/ifaddrss.c \
    Widgets/documentviewer.c \
    Logic/xdocument.c \
    Widgets/keycompose.c \
    Widgets/listview.c \
    Widgets/listviewitem.c \
    Widgets/s_treeview.c \
    Widgets/s_treeviewitem.c \
    Widgets/scrollbar.c \
    Widgets/slider.c \
    Widgets/splitter.c \
    Widgets/textline.c

HEADERS += \
    Logic/minilzo/lzoconf.h \
    Logic/minilzo/lzodefs.h \
    Logic/minilzo/minilzo.h \
    Logic/rc_client.h \
    Logic/rc_server.h \
    Logic/rc_server_form.h \
    Logic/shark.h \
    Logic/system.h \
    Widgets/button.h \
    Widgets/color.h \
    Widgets/events.h \
    Widgets/eventstack.h \
    Widgets/form.h \
    Widgets/graphics.h \
    Widgets/label.h \
    Widgets/panel.h \
    Widgets/widget.h \
    eventsqueue.h \
    public.h \
    Logic/ifaddrss.h \
    Widgets/documentviewer.h \
    Logic/xdocument.h \
    Widgets/keycompose.h \
    Widgets/listview.h \
    Widgets/listviewitem.h \
    Widgets/s_treeview.h \
    Widgets/s_treeviewitem.h \
    Widgets/scrollbar.h \
    Widgets/slider.h \
    Widgets/splitter.h \
    Widgets/textline.h \
    Widgets/mainform.h

