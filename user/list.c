#include "list.h"

LISTNODE *node_create(void)
{
    LISTNODE *node = (LISTNODE *)malloc(sizeof(LISTNODE));
    node->prev = NULL;
    node->next = NULL;
    node->data = NULL;

    return node;
}

void node_insert(LISTNODE **head, void *data)
{
    LISTNODE *p = (LISTNODE *)malloc(sizeof(LISTNODE));

    p->data = data;
    p->next = (*head);
    p->prev = NULL;
    (*head)->prev = p;
    (*head) = p;
}

void node_delete(LISTNODE **head, LISTNODE *p)
{
    if (p->prev) {
        p->prev->next = p->next;
        p->next->prev = p->prev;
    } else {
        (*head) = p->next;
        p->next->prev = NULL;
    }

    free(p->data);
    free(p);
}

LISTNODE *node_next(LISTNODE *head)
{
    LISTNODE *p = head;

    if (p != NULL) {
        return p->next;
    }
}

LISTNODE *node_search_cmd(LISTNODE *head, int n)
{
    LISTNODE *p = head;

    if (n == 0) {
        return NULL;
    }

    while(p) {
        CMD_DATA *data = (CMD_DATA *)p->data;
        if (data != NULL) {
            if (n == data->fd) {
                return p;
            }
        }
        p = p->next;
    }

    return NULL;
}

void node_display_cmd(LISTNODE *head)
{
    int cnt = 0;
    LISTNODE *p = head;

    fprintf(stderr, "\r\n fd   host             port \r\n");

    while(p) {
        CMD_DATA *data = (CMD_DATA *)p->data;
        if (data != NULL) {
          fprintf(stderr, " ");
          fprintf(stderr, "%-5d", data->fd);
          fprintf(stderr, "%-17s", data->host);
          fprintf(stderr, "%-7d", data->port);
          fprintf(stderr, "\r\n");
          cnt++;
        }
        p = p->next;
    }
    fprintf(stderr, " <%d> \r\n\r\n", cnt);
}
