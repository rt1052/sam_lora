#ifndef _LORA_H_
#define _LORA_H_



#define SET_RELAY_REQUEST   0x1
#define SET_RELAY_RESPONSE  0x2
#define GET_RELAY_REQUEST   0x3
#define GET_RELAY_RESPONSE  0x4
#define GET_PARAM_REQUEST   0x5
#define GET_PARAM_RESPONSE  0x6
#define SET_RELAY_NOTICE    0x7

typedef struct {
	uint8_t msg_buf[10][64];
    uint8_t msg_cnt;

	uint8_t msg_send_buf[10][64];
    uint8_t msg_send_cnt;    
} lora_st;

extern lora_st lora;


void *thread_lora(void *arg);
void *thread_lora_recv(void *arg);
void *thread_lora_send(void *arg);






#endif
