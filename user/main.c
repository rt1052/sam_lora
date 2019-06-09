#include "main.h"
#include "lora.h"
#include "tcp.h"

int lora_msg;
sem_t lora_sem;

pthread_mutex_t lora_lock;
pthread_cond_t lora_cond;

int main(int argc, char **argv)
{
    int ch;
    pthread_t tcp_thread, lora_thread, user_thread, gets_thread, data_thread;
    void *thread_result;
    int res;

    printf("\r\n************** welcome *************** \r\n");
    while ((ch = getopt(argc, argv, "s:h")) != -1) {
        switch (ch) {
            case 's':
                // printf("The argument of -l is %s\n\n", optarg);
                //sscanf(optarg, "%d", &min_sz);
                break;
            case 'h':
            case '?':
                printf("\r\nUsage: network [options] \r\n");
                printf("    -h  this help \n\r");
                printf("    -s  set minimum buf size \r\n");
                printf("\r\n");
                return -1;
        }
    }

    log_write("reboot");

    pthread_mutex_init(&lora_lock, NULL);
    pthread_cond_init(&lora_cond,NULL); 
    sem_init(&lora_sem, 0, 0);

    /* create msg queue */
    lora_msg = msgget((key_t)6523, 0666 | IPC_CREAT);
    if (lora_msg == -1) {
        printf("create msg queue error \r\n");
        exit(EXIT_FAILURE);
    }

    /* tcp thread */
    res = pthread_create(&tcp_thread, NULL, thread_tcp, NULL);
    if (res != 0) {
        printf("tcp thread creation failed");
        exit(EXIT_FAILURE);
    }  
    /* lora rx&tx */
    res = pthread_create(&lora_thread, NULL, thread_lora, NULL);
    if (res != 0) {
        printf("lora thread creation failed");
        exit(EXIT_FAILURE);
    }
    /* user input */
    res = pthread_create(&user_thread, NULL, thread_user, NULL);
    if (res != 0) {
        fprintf(stderr, "user thread creation failed");
        exit(EXIT_FAILURE);
    }

    /* gets thread */
    res = pthread_create(&gets_thread, NULL, thread_gets, NULL);
    if (res != 0) {
        fprintf(stderr, "gets thread creation failed");
        exit(EXIT_FAILURE);
    }
    /* time ctrl */

    /* data base */
    res = pthread_create(&data_thread, NULL, thread_data, NULL);
    if (res != 0) {
        fprintf(stderr, "data thread creation failed");
        exit(EXIT_FAILURE);
    }
    /* search device */

    /* wait for thread finish */
    res = pthread_join(lora_thread, &thread_result);
    if (res != 0) {
        printf("lora thread join failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_join(tcp_thread, &thread_result);
    if (res != 0) {
        printf("tcp thread join failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


uint8_t check_sum(uint8_t *buf, uint16_t len) 
{
    uint16_t i;
    uint8_t sum = 0;

    for(i = 0; i < len; i++) {
        sum += buf[i];
    }

    return sum;        
}
