#include "main.h"
#include "list.h"
#include "tcp.h"
#include "lora.h"
#include "alarm.h"


LISTNODE *alarm_node_head, **alarm_node_head_p;

void *thread_alarm(void *arg)
{
    /* wait for other thread */
    sleep(3);

    /* create alarm list */
    alarm_node_head = node_create();
    alarm_node_head_p = &alarm_node_head;

    while(1) {
        LISTNODE *node = alarm_node_head;
        while(node->data) {
            ALARM_DATA *alarm_data = (ALARM_DATA *)node->data;
            if (time(NULL) > alarm_data->time) {
                if ((time(NULL)-alarm_data->time) < alarm_data->timeout) {
                    pthread_mutex_lock(&lora_send_lock); 
                    lora_send(alarm_data->fd, alarm_data->id, alarm_data->cmd, alarm_data->dat);
                    pthread_mutex_unlock(&lora_send_lock); 
                } else {
                    if (alarm_data->interval) {
                        log_write("alarm[%d] time out \n", alarm_data->fd);
                        alarm_data->time += alarm_data->interval;
                    } else {
                        node_delete(alarm_node_head_p, node);
                    }
                }
            } 
            node = node_next(node);
        }
        sleep(5); 
    }
}

void *thread_alarm_event(void *arg)
{
    struct tm t;

	sleep(4);

    CMD_DATA *cmd_data = (CMD_DATA *)malloc(sizeof(CMD_DATA));
    cmd_data->fd = 2;
    cmd_data->port = 2;
    strcpy(cmd_data->host, "local-alarm");
    cmd_data->active = true;
    memset(cmd_data->send_buf, 0, 128);
    sem_init(&cmd_data->sem, 0, 0);
    node_insert(node_head_p, cmd_data);

    /* add alarm event */
    ALARM_DATA *alarm_data = (ALARM_DATA *)malloc(sizeof(ALARM_DATA));
    strptime("1970-01-01 07:57:00", "%Y-%m-%d %H:%M:%S", &t);
    /* one day */
    uint32_t interval = 60 * 60 * 24;
    alarm_data->time = mktime(&t) + time(NULL)/interval*interval;
    alarm_data->interval = interval;
    alarm_data->timeout = 60 * 10;
    alarm_data->fd = cmd_data->fd;
    alarm_data->id = 1;
    alarm_data->cmd = SET_RELAY_REQUEST;
    alarm_data->dat = 0;
    node_insert(alarm_node_head_p, alarm_data);    

	while(1) {
		int res = sem_trywait(&cmd_data->sem);
        /* received sem */
        if (res == 0) {
            uint8_t fd = cmd_data->send_buf[2];
            uint8_t id = cmd_data->send_buf[3];
            uint8_t dat = cmd_data->send_buf[5];

            if (alarm_data->fd == fd) {
                printf("1 \n");
                LISTNODE *node = node_search_alarm(alarm_node_head, id);
                if (node != NULL) {
                    alarm_data = (ALARM_DATA *)node->data;
                    if (time(NULL) > alarm_data->time) {
                        printf("2 \n");
                        if (alarm_data->dat == dat) {
                            printf("3 \n");
                            if (alarm_data->interval) {     
                                printf("########### \n");                   
                                alarm_data->time += alarm_data->interval;
                            } else {
                                node_delete(alarm_node_head_p, node);
                            }
                        }
                    }
                }  
            }             
        } else {
            usleep(100);
        }  
	}
}

