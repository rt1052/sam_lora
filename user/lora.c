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

int lora_send(uint8_t port, uint8_t id, uint8_t cmd, uint8_t *dat, uint8_t cnt)
{
    uint8_t buf[64];
    uint8_t len = cnt + 5;
    
    buf[0] = 0x42;
    buf[1] = len;
    buf[2] = port;
    buf[3] = id;
    buf[4] = cmd;
    memcpy(buf+5, dat, cnt);
    buf[len] = check_sum(buf, len);
    
    int res = SX1276GetRFState();
    //printf("res = %d \r\n", res);
    if ((res == 2)) { //} || (res == 0)) {  // RFLR_STATE_RX_RUNNING
        SX1276SetTxPacket(buf, len+1);  
        return 0;
    } else {
        return -1;
    }
}

void *thread_lora(void *arg)
{
    uint8_t rx_buf[128];
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
            // fprintf(stderr, "timeout \r\n");        
            break;
        case RF_RX_DONE:
            //printf("gain = %d \r\n", SX1276LoRaGetPacketRxGain());

            SX1276GetRxPacket(rx_buf, &len);
#if 0            
            fprintf(stderr, "recv: \r\n");
            for(uint8_t i = 0; i < len; i++) {
                fprintf(stderr, "0x%02x ", rx_buf[i]);
            }
            fprintf(stderr, "\r\n");
#endif

            if (rx_buf[len-1] == check_sum(rx_buf, len-1)) {

                uint8_t port = rx_buf[2];
                uint8_t cmd = rx_buf[4];
                if ((cmd == SET_RELAY_NOTICE) || 
                    (cmd == SET_RELAY_RESPONSE) || 
                    (cmd == GET_PARAM_RESPONSE)) {
                    LISTNODE *node = node_head;
                    while(node->data) {
                        CMD_DATA *data = (CMD_DATA *)node->data;
                        memcpy(data->send_buf, rx_buf, len);
                        sem_post(&data->sem);

                        node = node_next(node);
                    }                      
                } else {
                    LISTNODE * node = node_search_cmd(node_head, port);
                    if (node != NULL) {
                        CMD_DATA *data = (CMD_DATA *)node->data;
                        memcpy(data->send_buf, rx_buf, len);
                        sem_post(&data->sem);
                    }                    
                }
            }
            break;
        case RF_TX_DONE:
            //printf("tx done \r\n");
            SX1276StartRx();
            break;
        default:
            break;
        }
        usleep(10 * 1000);
    }
}
