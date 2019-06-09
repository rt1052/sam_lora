#include "main.h"
#include "list.h"
#include "tcp.h"

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
            } else if (0 == strncmp(getsBuf, "ls", strlen("ls"))) {
                node_display_cmd(node_head);
            }
#if 0
            else if (0 == strncmp(getsBuf, "on ", strlen("on "))) {
                sscanf(getsBuf+3, "%d", &id);
                pthread_mutex_lock(&lora_lock);
                fprintf(stderr, "lora lock \r\n");
                value = 1;
                lora_send(id, SET_RELAY_REQUEST, &value, 1);
                sem_wait(&lora_sem);
                if (lora_frame.cmd == SET_RELAY_RESPONSE) {
                    fprintf(stderr, "sys# set ok \r\n");
                }

                pthread_mutex_unlock(&lora_lock);
                fprintf(stderr, "lora unlock \r\n");

            } else if (0 == strncmp(getsBuf, "off ", strlen("off "))) {
                sscanf(getsBuf+4, "%d", &id);
                pthread_mutex_lock(&lora_lock);
                fprintf(stderr, "lora lock \r\n");
                value = 0;
                lora_send(id, SET_RELAY_REQUEST, &value, 1);
                sem_wait(&lora_sem);
                if (lora_frame.cmd == SET_RELAY_RESPONSE) {
                    fprintf(stderr, "sys# set ok \r\n");
                }

                pthread_mutex_unlock(&lora_lock);
                fprintf(stderr, "lora unlock \r\n");

            } else if (0 == strncmp(getsBuf, "get ", strlen("get "))) {
                sscanf(getsBuf+4, "%d", &id);
                pthread_mutex_lock(&lora_lock);

                value = 0;
                lora_send(id, GET_RELAY_REQUEST, &value, 1);
                sem_wait(&lora_sem);
                if (lora_frame.cmd == GET_RELAY_RESPONSE) {
                    fprintf(stderr, "sys# %s \r\n", lora_frame.dat[0] ? "on" : "off");
                }

                pthread_mutex_unlock(&lora_lock);                

            } else if (0 == strncmp(getsBuf, "dat ", strlen("dat "))) {
                sscanf(getsBuf+4, "%d", &id);
                pthread_mutex_lock(&lora_lock);

                value = 0;
                lora_send(id, GET_PARAM_REQUEST, &value, 1);
                sem_wait(&lora_sem);
                if (lora_frame.cmd == GET_PARAM_RESPONSE) {
                    fprintf(stderr, "sys# humi:%d, temp:%d.%d \r\n", 
                    lora_frame.dat[0], lora_frame.dat[2], lora_frame.dat[3]); 
                }

                pthread_mutex_unlock(&lora_lock);                

            } else if (0 == strcmp(getsBuf, "help")) {
                fprintf(stderr, " -on    switch on \r\n");
                fprintf(stderr, " -off   switch off \r\n");
                fprintf(stderr, " -get   get state \r\n");
                fprintf(stderr, " -dat   get dat \r\n");
                fprintf(stderr, " -help  this help \r\n");
            } else {
                fprintf(stderr, "unknow command \r\n");
            }
#endif            

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

//#include <stdlib.h>  
#include "sqlite3.h"  

void *thread_data(void *arg)
{
    char *recv_buf;
    uint8_t dat = 0;

    char str[128];
    sqlite3 *db=NULL;  
    char *err = 0;  
    int rc;  
    int res;

    int port = 1;

    msg_st msg;
    lora_frame_t *lora_frame;

    rc = sqlite3_open("dht11.db", &db);  
    if (rc) {  
        fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
    }  
    char *sql = " CREATE TABLE dht11(Time VARCHAR(12), humi REAL, temp REAL);" ;  
    sqlite3_exec( db , sql , 0 , 0 , &err );      

    sleep(5);

    while(1) {
        sleep(2);
    }

        CMD_DATA *data = (CMD_DATA *)malloc(sizeof(CMD_DATA));
        data->fd = 1;
        memset(data->send_buf, 0, 128);
        sem_init(&data->sem, 0, 0);

        node_insert(node_head_p, data);

    while(1) {
        lora_send(port, 1, GET_PARAM_REQUEST, &dat, 1);

        usleep(200 * 1000);
        res = sem_trywait(&data->sem);
        if (res == 0) {
            lora_frame = (lora_frame_t *)data->send_buf;
            if (lora_frame->cmd == GET_PARAM_RESPONSE) {
                //fprintf(stderr, "humi:%d, temp:%d.%d \r\n", 
                //        lora_frame.dat[0], lora_frame.dat[2], lora_frame.dat[3]); 
                        
                sprintf(str, "INSERT INTO \"dht11\" VALUES(datetime('now'), %d, %d.%d);", 
                        lora_frame->dat[0], lora_frame->dat[2], lora_frame->dat[3]);
                sqlite3_exec(db, str, 0, 0, &err);  
            } 
            memset(data->send_buf, 0, 128);
        }
        sleep(5 * 60);
    }

    sqlite3_close(db); 
}