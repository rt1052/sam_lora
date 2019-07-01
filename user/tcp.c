#include "tcp.h"
#include "main.h"
#include "list.h"
#include "lora.h"
#include "list.h"

int fd_tcp_cli;

int talkFlag;

LISTNODE *node_head, **node_head_p;

/* server recv data from client */
void *thread_client(void *arg)
{
    char buf[128];
    int res;

    LISTNODE *node = (LISTNODE *)arg;
    CMD_DATA *data = (CMD_DATA *)node->data;

    log_write("%d connected \r\n", data->fd);

    //uint8_t dat = 0;
    //lora_send(data->fd, 1, GET_PARAM_REQUEST, &dat, 1);

    while(1) {
        int len = recv(data->fd, buf, sizeof(buf), 0);  //MSG_WAITALL

        if (len > 0) {
            if (1) { //(buf[len-1] == check_sum(buf, len-1)) {

                uint8_t id = buf[2];
                uint8_t cmd = buf[3];
                uint8_t dat = buf[4];

                log_write("tcp len = %d \r\n", len);
                res = lora_send(data->fd, id, cmd, &dat, 1);
                if (res == -1) {
                    // send(data->fd, data->send_buf, len, 0);
                    log_write("#lora busy \r\n");
                }
            }          
        } else if (len == -1) {  /* timeout */
            usleep(100);
        } else {  /* len == 0 client disconnected */
            log_write("%d disconnected \r\n", data->fd);
            break;
        }

        res = sem_trywait(&data->sem);
        /* received sem */
        if (res == 0) {
            len = data->send_buf[1];
            log_write("lora len = %d \r\n", len);
            send(data->fd, data->send_buf, len, 0);
            memset(data->send_buf, 0, 128);
        }
    }

    close(data->fd);
    sem_destroy(&data->sem);
    node_delete(node_head_p, node);
}

void *thread_tcp(void *arg)
{
    int res;
    struct sockaddr_in server, client;

    pthread_t client_thread;
    void *thread_result;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(TCP_PORT);

    /* create socket */
    int fd_tcp_srv = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(fd_tcp_srv, SOL_SOCKET, SO_REUSEADDR, NULL, 1);
    bind(fd_tcp_srv, (struct sockaddr *)&server, sizeof(server));
    listen(fd_tcp_srv, 5);

    /* create node */
    node_head = node_create();
    node_head_p = &node_head;

    while(1) {
        int len = sizeof(struct sockaddr);
        fd_tcp_cli = accept(fd_tcp_srv, (struct sockaddr *)&client,
                            (socklen_t *)&len);

        struct timeval timeout = {0, 200 * 1000};
        setsockopt(fd_tcp_cli, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(struct timeval));

        CMD_DATA *data = (CMD_DATA *)malloc(sizeof(CMD_DATA));
        data->fd = fd_tcp_cli;
        data->active = true;
        strcpy(data->host, inet_ntoa(client.sin_addr));
        data->port = ntohs(client.sin_port);
        memset(data->send_buf, 0, 128);
        sem_init(&data->sem, 0, 0);

        node_insert(node_head_p, data);

        /* create thread */
        res = pthread_create(&client_thread, NULL, thread_client, node_head);
        if (res != 0) {
            log_write("client thread creation failed \r\n");
            exit(EXIT_FAILURE);
        }
    }
    close(fd_tcp_srv);

    pthread_exit("tcp thread end");
}
