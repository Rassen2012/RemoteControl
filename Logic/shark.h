#ifndef SHARK_H
#define SHARK_H

#include <pthread.h>
#include <arpa/inet.h>

#define isvalidsock(s) ((s) >= 0)
#define SHARK_BUFFSIZE 4096//10000//65536
/* �������� ������� ������ */

typedef void (*shark_receive) (int addr,char *buf,int size);

/* -------------------------------------------------------------------------------------------- */
/* ���� �� ���-�� ���������. � ������������ ����������������� ������� ��������� */

typedef struct shark
{
  /* ���� �������� */
  int             portsnd;
  /* ���� ������ */
  int             portrcv;
  /* ����� ������ */
  int             sockrcv;
  /* ����� � ������ ������ ������������ */
  char               buf[SHARK_BUFFSIZE];
  /* ����� �������� */
  int            socksend;
  /* ���� ���� �������-������� ������ ������� � ����� */
  pthread_t     thrserver;
  /* ������� ����������� ��� ������ */
  struct sockaddr_in addr;
  /* ������� ������� ������ */
  shark_receive rcv;

  int size;

  int state;
  
  char srv;

  char *cl_addr;
  
} shark;

/* -------------------------------------------------------------------------------------------- */
/* �������� ���������� data �������� size �� ��������, ������� ������ ��� �������� ����� */

int shark_send (shark *sh, char * data, int size);

/* -------------------------------------------------------------------------------------------- */
/* �������� ����� ����. port ������ ���� ������, ip-���� ����� ����������� 
   (255-���������������), buffsize - ������ ������ ������-��������, 
   rcv-��������� �� ������� ���������� �������� ������ */

shark *shark_init (int portsnd, int portrcv, char *ip, shark_receive rcv);

/* -------------------------------------------------------------------------------------------- */
/* ��������� �� ������ ����� � ����� ����� ����� ��������� �����������. ���������� ���������� 
   ����������� ����.��������� ��� �����, ���������� � ������� shark_init � ��������� ���������� NULL  */

char *shark_read (shark *sh,int *addr,int *count);
int shark_send_all(shark *sh, char *data, int size);
int shark_recv_all(shark *sh, char *buf, int size);
void shark_close(shark *sh);
char *shark_getHostAddr(struct in_addr s_addr);
int shark_getLocalAddr();

#endif
