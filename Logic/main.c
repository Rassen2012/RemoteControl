#include <stdio.h>
#include <malloc.h>
//#include <X11/Xutil.h>
#include "rc_server.h"
#include "rc_client.h"
#include "system.h"
#include "../Widgets/form.h"


int main(int argc, char **argv){
    //Display *d = XOpenDisplay(NULL);
    //Window rootWindow = XRootWindow(d, XDefaultScreen(d));
    //Temp_Function(d, rootWindow);
    //XCloseDisplay(d);
    if(argc > 1){
        RCClient_Start("Linux Mint");
        //RCClient_Start(argv[1]);
        while(1) thread_pause(1000);
    }
	else{
		RCServer_Start();
	}
	return 0;
}
