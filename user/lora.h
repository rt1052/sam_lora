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


typedef struct {
    uint8_t head;
    uint8_t len;
    uint8_t port;
    uint8_t id;
    uint8_t cmd;
    uint8_t dat[4];
    // uint8_t cs;
} lora_frame_t;

extern lora_st lora;

void lora_send(uint8_t fd, uint8_t id, uint8_t cmd, uint8_t dat);
void *thread_lora(void *arg);
void *thread_lora_recv(void *arg);
void *thread_lora_send(void *arg);



/*  lora frame
bit   info
0     head
1     len
2     fd
3     id
4     cmd
5     dat

n     cs
*/


#endif
