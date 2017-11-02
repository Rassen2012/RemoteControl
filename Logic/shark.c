#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include "ifaddrss.h"

#include <sys/time.h>
#include "public.h"
#include "shark.h"
#include "system.h"

#define LOCAL_HOST "127.0.0.1"

int shark_local_addr = 0;

/* -------------------------------------------------------------------------------------------- */
/* Установка адреса */

void shark_set_address(char *hname, short port,
                       struct sockaddr_in *sap)
{
    //struct servents *sp;
    struct hostent *hp;

    //printf("shark: set addr host %s  port %d\n", hname, port);
    bzero(sap, sizeof(*sap));
    sap->sin_family = AF_INET;
    if (hname != NULL)
    {
        if (!inet_aton(hname, &sap->sin_addr))
        {
            hp = gethostbyname(hname);
            if (hp == NULL)
                perror("shark: unknown host");
            sap->sin_addr = *(struct in_addr *)hp->h_addr;
        }
    }
    else
        sap->sin_addr.s_addr = htonl(INADDR_ANY);
    sap->sin_port = htons(port);
}

/* -------------------------------------------------------------------------------------------- */
/* Создание сокета на прием */

int shark_udp_server(char *hname, short port)
{
    int s;
    struct sockaddr_in local;

    const int on = 1;

    shark_set_address(hname, port, &local);//, "udp");
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (!isvalidsock(s)){
        perror("shark: socket error");
        return -1;
    }

    if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on)))
        perror("shark: setsockopt error ");

    if (bind(s, (struct sockaddr*)&local, sizeof(local))){
        perror("shark: bind error");
        return -1;
    }
    return s;

}

/* -------------------------------------------------------------------------------------------- */
/* Открыть клиентский сокет */

int shark_udp_client(char *hname, short port, struct sockaddr_in *sap)
{
    int s;
    shark_set_address(hname, port, sap);//, "udp");
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (!isvalidsock(s))
    {
        perror("shark: socket error");
        return -1;
    }
    return s;
}

/* -------------------------------------------------------------------------------------------- */
/* Сервер юдипи сети  */

void *shark_server(void * arg)
{
    shark *sh=(shark *)arg;
    int s, len;
    socklen_t sa_size;
    struct sockaddr_in client_addr;
    /* Буфер для передачи сообщений */
    char buf[SHARK_BUFFSIZE];
    /* Создаем сокет для чтения данных */
    s = shark_udp_server(0, sh->portrcv);
    if (s == -1) { printf("shark: no info server\n"); return NULL;}

    struct timeval time_count;
    fd_set read_fds, write_fds;
    int n;
    /* Крутимся до победы */
    while(sh->srv)
    {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_SET(s, &read_fds);
        sa_size = sizeof(client_addr);
        time_count.tv_sec = 1;
        time_count.tv_usec = 0;
        n = select(s+1, &read_fds, &write_fds, 0, &time_count);
        if(n < 0){
            printf("Null !!!!!!\n");
            if(errno == EINTR) continue;
            break;
        }
        if(FD_ISSET(s, &read_fds)){
            /* Читаем данные из сокета */
            len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &sa_size);
            if (len < 1) continue;
            //printf ("sin_addr[0]=%x\n",client_addr.sin_addr);
            sh->cl_addr = shark_getHostAddr(client_addr.sin_addr);
            if (sh->rcv) sh->rcv (client_addr.sin_addr.s_addr>>24,buf,len);

            FD_CLR(s, &read_fds);
        }
        /*else if(time_count.tv_sec <= 0 && time_count.tv_usec <= 0){
        printf("time out, port - %d\n", sh->portsnd);
    }*/
    }
    close(s);
    return NULL;
}

/* -------------------------------------------------------------------------------------------- */
/* Прочитать из сокета шарка в буфер буфер шарка очередную дейтаграмму. Возвращает количество 
   прочитанных байт.Актуально для шарка, созданного с помощью shark_init с последним параметром NULL  */

char *shark_read (shark *sh,int *addr,int *count)
{
    struct sockaddr_in client_addr;
    socklen_t sa_size;
    int ret;
    fd_set read_fds, write_fds;
    int n;
    struct timeval time_count;
    unsigned long time_val;
    *count = 0;
    time_count.tv_sec = 1;
    time_count.tv_usec = 0;
    while(1){
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_SET(sh->sockrcv, &read_fds);
        //time_count.tv_sec = 4;
        //time_count.tv_usec = 0;
        //      printf("Select begin! Time val - %d sec, %d mlsec\n", (int)time_count.tv_sec, (int)time_count.tv_usec);
        time_val = time_get();
        n = select(sh->sockrcv+1, &read_fds, &write_fds, 0, &time_count);
        time_val = time_get() - time_val;
        //      printf("Select end! Time val - %d sec, %d mlsec\n", (int)time_count.tv_sec, (int)time_count.tv_usec);
        if(n < 0){
            if(errno == EINTR) continue;
            return NULL;
        }
        if(FD_ISSET(sh->sockrcv, &read_fds)){
            ret = recvfrom(sh->sockrcv, sh->buf, SHARK_BUFFSIZE, 0, (struct sockaddr *)&client_addr, &sa_size);
            if (addr) *addr=client_addr.sin_addr.s_addr>>24;
            *count = ret;
            FD_CLR(sh->sockrcv, &read_fds);
            return sh->buf;
        }
        else if(time_val >= 1000 || (time_count.tv_sec <= 0 && time_count.tv_usec <= 0)){
            //printf("shark: receive time out\n");
            return NULL;
        }
    }
}


/* -------------------------------------------------------------------------------------------- */
/* Создание юдипи сети. portsnd задает порт передачи, portrcv - порт приема, ip-айпи адрес передатчика 
   (255-широкоформатный, 0 - локальный),   rcv-указатель на функцию обработчик принятых данных.
   Если rcv задан NULL то нити чтения не создается и пользователь сам должен вычитывать из сокета
   с помощью функции shark_read. Если rcv задан, то чтение идет в отдельной нити. Пользователю
   по коллбэку приходят дейтаграммы  */

shark *shark_init (int portsnd,int portrcv,char *ip,shark_receive rcv)
{
    /* Выделяем память под возвращаемую структуру */
    shark *ret=calloc (1,sizeof (shark));
    /* Запоминаем параметры */
    ret->portsnd=portsnd;
    ret->portrcv=portrcv;
    ret->rcv=rcv;
    /* По-умолчанию считаем, что передаем только на себя (локалхост) */
    //char ips[32]="127.0.0.1";

    /* Формируем ай-пи передатчика */
    if (!ip) ip = LOCAL_HOST;//sprintf (ips,"192.9.2.%d",ip);
    /* Открываем сокет для передачи */
    ret->socksend = shark_udp_client (ip, portsnd, &ret->addr);
    /* Если адрес широкоформатный нужно установить у сокета атрибут */
    int on=1;
    int s = atoi(strchr(ip, '.'));
    if (s==255) setsockopt(ret->socksend, SOL_SOCKET, SO_BROADCAST,(char*)&on,sizeof(int));
    /* Запускаем нить чтения из порта, если задан режим с коллбэком*/
    if (rcv){
        ret->srv = 1;
#if defined(OC2K1x)
        pthread_attr_t attr;
        struct sched_param param;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        pthread_attr_getschedparam(&attr, &param);
        param.sched_priority = 100;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&ret->thrserver, &attr, shark_server, ret);
#else
        pthread_create (&ret->thrserver,NULL,shark_server,ret);
#endif
    }
    //  else ret->thrserver = NULL;
    /* Открываем сокет чтения, если задано чтение пользователя внешнее */
    else
    {
        //ret->thrserver = NULL;
        ret->srv = 0;
        /* Создаем сокет для чтения данных */
        ret->sockrcv = shark_udp_server(0, ret->portrcv);
        if (ret->sockrcv == -1)
        {
            //printf("shark: no info server\n");
            close (ret->socksend);
            free (ret);
            return NULL;
        }
        //ret->buf=malloc (SHARK_BUFFSIZE);
        //printf("Server create!\n");
    }
    /* Возвращаем созданный объект */
    return ret;
}

int shark_send_all(shark *sh, char *data, int size){
    int size_send = 0;
    int sd = 0;
    int ret = 0;
    char buf[SHARK_BUFFSIZE];
    //char *recv_buf;
    while(size_send < size){
        sd = (size - size_send) > SHARK_BUFFSIZE ? SHARK_BUFFSIZE : (size - size_send);
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &data[size_send], sd);
        ret = shark_send(sh, buf, sd);
        if(ret < sd){
            printf("shark_send_all error: try send %d, ret val %d", sd, ret);
            return size_send;
        }
        size_send += sd;
    }
    return size_send;
}

int shark_recv_all(shark *sh, char *buf, int size){
    int size_recv = 0;
    int ret = 0;
    //int ok = RECEIVE_OK;
    int fail = RECEIVE_FAILED;
    char *recv_buf;
    char send_buf[sizeof(int)];
    while(size_recv < size){
        recv_buf = shark_read(sh, 0, &ret);
        if(ret < 1){
            memcpy(send_buf, &fail, sizeof(int));
            shark_send(sh, send_buf, sizeof(int));
            printf("shark_recv_all error: receive bad buf\n");
            return size_recv;
        }
        memcpy(&buf[size_recv], recv_buf, ret);
        //memcpy(send_buf, &ok, sizeof(int));
        //shark_send(sh, send_buf, sizeof(int));
        size_recv += ret;
    }
    return size_recv;
}

/* -------------------------------------------------------------------------------------------- */
/* Передать кодограмму data размером size на адресата, который указан при создании шарка */

int shark_send (shark *sh, char * data, int size)
{
    if (sh->socksend<0) return -1;
    while(sendto(sh->socksend, data, size, 0, (struct sockaddr *)&sh->addr, sizeof(sh->addr)) < size) thread_pause(1);
    return size;//(sendto(sh->socksend, data, size, 0, (struct sockaddr *)&sh->addr, sizeof (sh->addr));
}

/* -------------------------------------------------------------------------------------------- */


char *shark_getHostAddr(struct in_addr s_addr){
    return inet_ntoa(s_addr);
}

int shark_getLocalAddr(){
    if(shark_local_addr > 0) return shark_local_addr;
    struct ifaddrs *ifadds, *ifa;
    int addr = 0;
    unsigned int localhost = (1<<24)|127;
    if(getifaddrs(&ifadds) != 0) return addr;
    struct sockaddr_in *sa;
    ifa = ifadds;
    do{
        if(ifa->ifa_addr == NULL) continue;
        if(ifa->ifa_addr->sa_family != AF_INET) continue;
        sa = (struct sockaddr_in*)ifa->ifa_addr;
        if(sa->sin_addr.s_addr > 0){
            if((unsigned int)sa->sin_addr.s_addr == localhost) continue;
            shark_local_addr = addr = sa->sin_addr.s_addr>>24;
            break;
        }
    }while((ifa = ifa->ifa_next) != NULL);

    freeifaddrs(ifadds);
    return addr;
}

#if 1

/* -------------------------------------------------------------------------------------------- */

//void rcv_from_shark (int addr,char *buffer,int len)
//{
//  printf ("rcv_from_shark=addr=%d '%s'\n",addr,buffer);
//}

/* -------------------------------------------------------------------------------------------- */

void *shark_thread_poll (void *par)
{
    int tmp;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &tmp);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &tmp);
    shark *s=(shark *)par;
    sleep (4);
    while (1)
    {
        int count;
        int addr;
        char *rcv=shark_read (s,&addr,&count);
        if (count>=0)
            printf ("shark_thread_poll addr=%d count=%d str='%s'\n",addr,count,rcv);
    }
    return NULL;
}

void shark_close(shark *sh){
    if(sh == NULL) return;
    //int ps, pr;
    //ps = sh->portsnd;
    //pr = sh->portrcv;
    //printf("Shark begin close port send - %d, port recv - %d\n", ps, pr);
    //    close(sh->sockrcv);
    //    close(sh->socksend);
    if(sh->srv) {sh->srv = 0; pthread_join(sh->thrserver, NULL);}
    close(sh->sockrcv);
    close(sh->socksend);
    //sh->sockrcv = -1;
    free(sh);
    //printf("Shark close port send - %d, port recv - %d\n", ps, pr);
    sh = NULL;
}

#define WITH_THREAD

//int main (int argc,char *argv[])
//{
//  if (argc<4)
//  {
//    printf ("usage %s <portsnd> <portrcv> <ip>  (255 for broadcast,0-for local host)\n",argv[0]);
//    return 0;
//  }
//  shark *s=shark_init (atoi (argv[1]),atoi (argv[2]),atoi (argv[3]),
//#ifdef WITH_THREAD
//  rcv_from_shark);
//#else
//  NULL);
//  pthread_t thr;
//  pthread_create (&thr,NULL,shark_thread_poll,s);
//#endif
//  char buf[1024];
//  while (1)
//  {
//    int ret;
//    buf[0]=0;
//    gets (buf);
//    ret=shark_send (s,buf,strlen (buf)+1);
//    printf ("send message len=%d ret=%d\n",strlen (buf)+1,ret);
//  }
//}

/* -------------------------------------------------------------------------------------------- */

#endif

/* -------------------------------------------------------------------------------------------- */
