#include <stdio.h>
#include <sys/time.h>
#include <time.h>

void thread_pause( long int mlsec )
{
    int  sign = 0;
    struct timespec rqtp;
    if( !mlsec ) return;
    rqtp.tv_sec    = (long int)(mlsec/1000);
    rqtp.tv_nsec  = mlsec*1000000 - rqtp.tv_sec*1000000000;
    sign = nanosleep( &rqtp, NULL );
}

//------------------------------------------------------------------------------------------------------

unsigned long time_get( )
{
    int                      sign;   // признак завершения какой-либо функции(успешно/неуспешно)
    struct timeval     t_current;   //
    struct timezone*    z_current;  //
    unsigned long        mlsec;   //

    z_current = NULL;
    sign =  gettimeofday( &t_current, z_current );

    mlsec = (t_current.tv_sec) * 1000 + t_current.tv_usec / 1000;
    return  mlsec;
}
