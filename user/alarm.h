#ifndef _ALARM_H_
#define _ALARM_H_


typedef struct {
    uint32_t time;
    uint32_t interval;
    uint16_t timeout;
    uint8_t fd;
    uint8_t id;
    uint8_t cmd;
    uint8_t dat;
} ALARM_DATA;

extern LISTNODE *alarm_node_head, **alarm_node_head_p;


void *thread_alarm(void *arg);
void *thread_alarm_event(void *arg);
















#endif
