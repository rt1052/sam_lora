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
	uint8_t buf[100];
    uint8_t len;
} lora_st;




void *thread_lora(void *arg);








#endif
