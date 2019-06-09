#ifndef _TCP_H_
#define _TCP_H_



#include "list.h"


#define TCP_PORT      54300


extern LISTNODE *node_head, **node_head_p;


void *thread_tcp(void *arg);
void *thread_tcp_server(void *arg);























#endif
