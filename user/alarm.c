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

void alarm_create(char *time_str, uint32_t interval, uint16_t timeout,
                  uint8_t fd, uint8_t id, uint8_t cmd, uint8_t dat)
{
    struct tm time_tm;

    ALARM_DATA *alarm_data = (ALARM_DATA *)malloc(sizeof(ALARM_DATA));
    if (time_str != NULL) {
        strptime(time_str, "%Y-%m-%d %H:%M:%S", &time_tm);
        alarm_data->time = mktime(&time_tm) + time(NULL)/(24*60*60)*(24*60*60);
    } else {
        alarm_data->time = time(NULL);
    }
    alarm_data->interval = interval;
    alarm_data->timeout = timeout;
    alarm_data->fd = fd;
    alarm_data->id = id;
    alarm_data->cmd = cmd;
    alarm_data->dat = dat;
    node_insert(alarm_node_head_p, alarm_data);      
}

void *thread_alarm_event(void *arg)
{
	sleep(4);

    CMD_DATA *cmd_data = (CMD_DATA *)malloc(sizeof(CMD_DATA));
    cmd_data->fd = 0x80;
    cmd_data->port = 0x80;
    strcpy(cmd_data->host, "local-alarm");
    cmd_data->active = true;
    memset(cmd_data->send_buf, 0, 128);
    sem_init(&cmd_data->sem, 0, 0);
    node_insert(node_head_p, cmd_data);

    /*              time                  interval  timeout fd                id   cmd                dat */
    alarm_create("1970-01-01 18:35:00", 10*60,     10,    cmd_data->fd+0x1, 0x1, SET_RELAY_REQUEST, 0x0);
    alarm_create("1970-01-01 18:40:00", 10*60,     10,    cmd_data->fd+0x2, 0x1, SET_RELAY_REQUEST, 0x1);
    alarm_create("1970-01-01 22:00:00", 60*60*24, 60*10,  cmd_data->fd+0x3, 0x2, SET_RELAY_REQUEST, 0x1);
    alarm_create("1970-01-01 08:00:00", 60*60*24, 60*10,  cmd_data->fd+0x4, 0x2, SET_RELAY_REQUEST, 0x0);

    alarm_create("1970-01-01 23:24:00", 0       , 60*10,  cmd_data->fd+0x5, 0x1, SET_RELAY_REQUEST, 0x1);

	while(1) {
		int res = sem_trywait(&cmd_data->sem);
        /* received sem */
        if (res == 0) {
            uint8_t fd = cmd_data->send_buf[2];
            uint8_t id = cmd_data->send_buf[3];
            uint8_t dat = cmd_data->send_buf[5];
                
            LISTNODE *node = node_search_alarm(alarm_node_head, fd);
            if (node != NULL) {
                ALARM_DATA *alarm_data = (ALARM_DATA *)node->data;
                if (time(NULL) > alarm_data->time) {
                    if (alarm_data->dat == dat) {
                        if (alarm_data->interval) {                   
                            alarm_data->time += alarm_data->interval;
                        } else {
                            node_delete(alarm_node_head_p, node);
                        }
                    }
                }
            }              
        } else {
            usleep(100);
        }  
	}
}

