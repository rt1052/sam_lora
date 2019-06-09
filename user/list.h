#ifndef _LIST_H_
#define _LIST_H_

#include "main.h"

typedef struct {
    int fd;
    bool active;
    char host[100];
    int port;
    sem_t sem;
    uint8_t recv_buf[128];
    uint8_t send_buf[128];
} CMD_DATA;

typedef struct node {
    void *data;
    struct node *prev;
    struct node *next;
} LISTNODE;



LISTNODE *node_create(void);
void node_insert(LISTNODE **head, void *data);
void node_delete(LISTNODE **head, LISTNODE *p);
LISTNODE *node_next(LISTNODE *head);
LISTNODE *node_search(LISTNODE *head, int n);
void node_display(LISTNODE *head);


#endif
