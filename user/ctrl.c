#include "main.h"
#include "list.h"
#include "tcp.h"
#include "alarm.h"

#define SET_RELAY_REQUEST   0x1
#define SET_RELAY_RESPONSE  0x2
#define GET_RELAY_REQUEST   0x3
#define GET_RELAY_RESPONSE  0x4
#define GET_PARAM_REQUEST   0x5
#define GET_PARAM_RESPONSE  0x6

bool getsFlag;
char getsBuf[128];


void *thread_user(void *arg)
{
    char buf[1024];
    msg_st msg;
    msg.type = 1;
    char str[5];    
    uint8_t value = 0;
    int id = 0;

    fprintf(stderr, "user# ");
    while(1) {
        if (true == getsFlag) {
            getsFlag = false;

            if (0 == strcmp(getsBuf, "")) {
                /* just press enter */
            } else if (0 == strncmp(getsBuf, "lsa", strlen("lsa"))) {
                node_display_alarm(alarm_node_head);
            } else if (0 == strncmp(getsBuf, "ls", strlen("ls"))) {
                node_display_cmd(node_head);
            } else if (0 == strcmp(getsBuf, "help")) {
                fprintf(stderr, " -ls    list client \r\n");
                fprintf(stderr, " -lsa   list alarm \r\n");
                fprintf(stderr, " -help  this help \r\n");
            } else {
                fprintf(stderr, "unknow command \r\n");
            }       

            fprintf(stderr, "user# ");
        } else {
            usleep(1000);
        }
    }
}

void *thread_gets(void *arg)
{
    while(1) {
        gets(getsBuf);
        getsFlag = true;
    }
}

void *thread_data(void *arg)
{
    uint8_t cnt = 0;

    char str[128];
    sqlite3 *db=NULL;  
    char *err = 0;  
    int res;

    sleep(5);

    /* sqlite init */
    res = sqlite3_open("dht11.db", &db);  
    if (res) {  
        fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
    }  
    char *sql = " CREATE TABLE dht11(Time VARCHAR(12), humi REAL, temp REAL);" ;  
    sqlite3_exec( db , sql , 0 , 0 , &err );      

    CMD_DATA *cmd_data = (CMD_DATA *)malloc(sizeof(CMD_DATA));
    cmd_data->fd = 1;
    cmd_data->port = 1;
    strcpy(cmd_data->host, "local-ctrl");
    cmd_data->active = true;
    memset(cmd_data->send_buf, 0, 128);
    sem_init(&cmd_data->sem, 0, 0);
    node_insert(node_head_p, cmd_data);
#if 1
    /* add alarm event */
    ALARM_DATA *alarm_data = (ALARM_DATA *)malloc(sizeof(ALARM_DATA));
    uint32_t interval = 2 * 60;
    alarm_data->time = time(NULL)/interval*interval;
    alarm_data->interval = interval;
    alarm_data->timeout = 60;
    alarm_data->fd = cmd_data->fd;
    alarm_data->id = 1;
    alarm_data->cmd = GET_PARAM_REQUEST;
    alarm_data->dat = 0;
    node_insert(alarm_node_head_p, alarm_data);       
#endif
    while(1) {
        res = sem_trywait(&cmd_data->sem);     
        if (res == 0) {
            uint8_t id = cmd_data->send_buf[3];
            uint8_t cmd = cmd_data->send_buf[4];
            uint8_t *dat = cmd_data->send_buf+5;

            LISTNODE *node = node_search_alarm(alarm_node_head, id);
            if (node != NULL) {
                alarm_data = (ALARM_DATA *)node->data;
                if (time(NULL) > alarm_data->time) {
                    if ((alarm_data->cmd+1) == cmd) {
#if 0                
                        log_write("humi:%d, temp:%d.%d \r\n", 
                                lora_frame->dat[0], lora_frame->dat[2], lora_frame->dat[3]); 
#else
                        /* default timezone is utc, we need localtime */
                        sprintf(str, "INSERT INTO \"dht11\" VALUES(datetime('now', 'localtime'), %d, %d.%d);", 
                                dat[0], dat[2], dat[3]);
                        sqlite3_exec(db, str, 0, 0, &err);  
#endif                                                             
                        if (alarm_data->interval) {  
                        printf("***********\n");                      
                            alarm_data->time += alarm_data->interval;
                        } else {
                            node_delete(alarm_node_head_p, node);
                        }
                    }
                }
            }   

#if 0
            if (cmd == GET_PARAM_RESPONSE) {

                //if (++cnt > 4) {
                    cnt = 0;
#if 0                
                    log_write("humi:%d, temp:%d.%d \r\n", 
                            lora_frame->dat[0], lora_frame->dat[2], lora_frame->dat[3]); 
#else
                    /* default timezone is utc, we need localtime */
                    sprintf(str, "INSERT INTO \"dht11\" VALUES(datetime('now', 'localtime'), %d, %d.%d);", 
                            dat[0], dat[2], dat[3]);
                    sqlite3_exec(db, str, 0, 0, &err);  
                //}
#endif                
            } 
#endif

            memset(cmd_data->send_buf, 0, 128);
        }

        usleep(100);
    }

    sqlite3_close(db); 
}



