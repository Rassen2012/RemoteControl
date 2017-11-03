#include <stdio.h>
#include "rc_server.h"
#include "rc_client.h"
#include "system.h"


int main(int argc, char **argv){
    if(argc == 1){
#ifdef LINUX_MINT
        RCClient_Start("Linux Mint");
#else
        RCClient_Start(argv[1]);
#endif
        while(1) thread_pause(1000);
    }
	else{
		RCServer_Start();
	}
	return 0;
}
