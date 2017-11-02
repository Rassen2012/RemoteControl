#ifndef SHARK_H
#define SHARK_H

#include <pthread.h>
#include <arpa/inet.h>

#define isvalidsock(s) ((s) >= 0)
#define SHARK_BUFFSIZE 4096//10000//65536
/* Описание функции приема */

typedef void (*shark_receive) (int addr,char *buf,int size);

/* -------------------------------------------------------------------------------------------- */
/* Сеть на юди-пи протоколе. С возможностью широковещательной посылки сообщений */

typedef struct shark
{
  /* Порт передачи */
  int             portsnd;
  /* Порт приема */
  int             portrcv;
  /* Сокет приема */
  int             sockrcv;
  /* Буфер в режиме чтения пользователя */
  char               buf[SHARK_BUFFSIZE];
  /* Сокет передачи */
  int            socksend;
  /* Нить юдпи сервера-который читает посылки с порта */
  pthread_t     thrserver;
  /* Адресок отправителя для сокета */
  struct sockaddr_in addr;
  /* Коллбэк функции приема */
  shark_receive rcv;

  int size;

  int state;
  
  char srv;

  char *cl_addr;
  
} shark;

/* -------------------------------------------------------------------------------------------- */
/* Передать кодограмму data размером size на адресата, который указан при создании шарка */

int shark_send (shark *sh, char * data, int size);

/* -------------------------------------------------------------------------------------------- */
/* Создание юдипи сети. port задает порт работы, ip-айпи адрес передатчика 
   (255-широкоформатный), buffsize - размер буфера приемо-передачи, 
   rcv-указатель на функцию обработчик принятых данных */

shark *shark_init (int portsnd, int portrcv, char *ip, shark_receive rcv);

/* -------------------------------------------------------------------------------------------- */
/* Прочитать из сокета шарка в буфер буфер шарка очередную дейтаграмму. Возвращает количество 
   прочитанных байт.Актуально для шарка, созданного с помощью shark_init с последним параметром NULL  */

char *shark_read (shark *sh,int *addr,int *count);
int shark_send_all(shark *sh, char *data, int size);
int shark_recv_all(shark *sh, char *buf, int size);
void shark_close(shark *sh);
char *shark_getHostAddr(struct in_addr s_addr);
int shark_getLocalAddr();

#endif
