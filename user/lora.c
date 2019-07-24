#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "platform.h"

#include "sx1276-Hal.h"
#include "radio.h"
#include "sx1276.h"

#include "main.h"
#include "list.h"
#include "tcp.h"
#include "lora.h"

extern int fd;

lora_st lora;

void lora_send(uint8_t fd, uint8_t id, uint8_t cmd, uint8_t dat)
{
    msg_st msg;
    uint8_t buf[64];
    uint8_t len = 6;
    
    buf[0] = 0x42;  /* head */
    buf[1] = len;
    buf[2] = fd;
    buf[3] = id;
    buf[4] = cmd;
    buf[5] = dat;
    buf[len] = check_sum(buf, len);

    /* create msg */
    msg.type = 1;
    uint8_t *ptr = lora.msg_send_buf + lora.msg_send_cnt;
    /* add checksum */
    memcpy(ptr, buf, len+1);
    msg.pdata = ptr;
    lora.msg_send_cnt = (++lora.msg_send_cnt) % 10;
    msgsnd(lora_send_msg, (void *)&msg, sizeof(msg.pdata), 0);
}

void *thread_lora_send(void *arg)
{
    msg_st msg;

    while(1) {
        msg.pdata = NULL;
        int res = msgrcv(lora_send_msg, (void *)&msg,
                         sizeof(msg.pdata), 0, IPC_NOWAIT);

        if (msg.pdata != NULL) {
            uint8_t *buf = msg.pdata;
            uint8_t len = buf[1]+1;

            //log_buf(buf, len);

            /* wait lora idle */
            while(SX1276GetRFState() != 0x2) {
                usleep(200*1000);
            }
            /* send lora data */
            SX1276SetTxPacket(buf, len);  
        }

        usleep(100);
    }
}

void *thread_lora_recv(void *arg)
{
    msg_st msg;

    while(1) {
        msg.pdata = NULL;
        int res = msgrcv(lora_recv_msg, (void *)&msg,
                         sizeof(msg.pdata), 0, IPC_NOWAIT);

        if (msg.pdata != NULL) {
            uint8_t *buf = msg.pdata;

            pthread_mutex_lock(&lora_lock); 

            uint8_t len = buf[1] + 1;
            uint8_t fd = buf[2];
            uint8_t cmd = buf[4];

            log_buf(buf, len);

            if ((cmd == SET_RELAY_NOTICE) || 
                (cmd == SET_RELAY_RESPONSE) || 
                ((cmd == GET_PARAM_RESPONSE) && 
                 (fd == 0))) {  /* only ctrl task data send to all fd */
                LISTNODE *node = node_head;
                while(node->data) {
                    CMD_DATA *data = (CMD_DATA *)node->data;
                    memcpy(data->send_buf, buf, len);
                    sem_post(&data->sem);

                    node = node_next(node);
                }                      
            } else {
                LISTNODE * node = node_search_cmd(node_head, fd);
                if (node != NULL) {
                    CMD_DATA *data = (CMD_DATA *)node->data;
                    memcpy(data->send_buf, buf, len);
                    sem_post(&data->sem);
                }                    
            }
            pthread_mutex_unlock(&lora_lock);     
        }
        usleep(100);
    }
}

void *thread_lora(void *arg)
{
    uint8_t rx_buf[1024];
    uint16_t len;
    msg_st msg;

    gpio_init(GPIO_RST);
    gpio_direction(GPIO_RST, 1);

    gpio_set_value(GPIO_RST, 0);
    usleep(500 * 1000);
    gpio_set_value(GPIO_RST, 1);

    fd = spi_init(0, 1, 400000);

    SX1276Init();
    SX1276StartRx();    

    while(1) {   
        switch(SX1276Process()) {          
        case RF_RX_TIMEOUT:      
            break;
        case RF_RX_DONE:
            SX1276GetRxPacket(rx_buf, &len);

            if ((len < 20) && (rx_buf[len-1] == check_sum(rx_buf, len-1))) {
                msg.type = 1;
                uint8_t *ptr = lora.msg_buf + lora.msg_cnt;
                memcpy(ptr, rx_buf, len);
                msg.pdata = ptr;
                lora.msg_cnt = (++lora.msg_cnt) % 10;
                msgsnd(lora_recv_msg, (void *)&msg, sizeof(msg.pdata), 0);
            } else {
                log_write("lora err len = %d \r\n", len);
            }
            break;
        case RF_TX_DONE:
            SX1276StartRx();
            break;
        default:
            break;
        }
        usleep(5 * 1000);
    }
}
