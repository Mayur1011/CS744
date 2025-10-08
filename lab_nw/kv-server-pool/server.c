#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include <pthread.h>

#define NO_OF_THREADS 10

typedef struct __kv_node {
    int key;
    char *value;
    struct __kv_node *next;
} kv_node_t;

// Useless btw
typedef struct {
    pthread_t tid;
    int is_busy;
    int thread_no;
} thread_info_t;

typedef struct __fd_queue {
    int client_fd;
    struct __fd_queue *next;
} fd_queue_t;

kv_node_t *kv_node_head = NULL;
kv_node_t *kv_node_tail = NULL;
pthread_mutex_t lock;
pthread_cond_t cond;
int available = 1;

thread_info_t thread_pool[NO_OF_THREADS];

fd_queue_t* fd_queue_head = NULL;
fd_queue_t* fd_queue_tail = NULL;
pthread_mutex_t fd_queue_lock;
pthread_cond_t fd_queue_cond;

void* handle_client(void* arg) {
    int thread_id = *(int*)arg;
    while(1) {
        pthread_mutex_lock(&fd_queue_lock);
        while(fd_queue_head == NULL) {
            pthread_cond_wait(&fd_queue_cond, &fd_queue_lock);
        }
        fd_queue_t* curr_fd_node = fd_queue_head;
        fd_queue_head = fd_queue_head->next;
        if (fd_queue_head == NULL) fd_queue_tail = NULL;
        curr_fd_node->next = NULL;
        pthread_mutex_unlock(&fd_queue_lock);
        int client_fd = curr_fd_node->client_fd;
        int client_disconnected = 0;
        printf("Thread %d is busy\n", thread_id);
        while (1)
        {
            if (client_disconnected) break;
            payload_t payload;
            bzero(&payload, sizeof(payload));
            ssize_t bytes_read = recv(client_fd, &payload, sizeof(payload), 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }

            switch (payload.cmd_type)
            {
                case CMD_CONNECT: 
                    {
                        char* msg = "Connected to server";
                        send(client_fd, msg, strlen(msg), 0);
                        break;
                    }
                case CMD_CREATE:
                    {
                        pthread_mutex_lock(&lock);
                        while(available == 0) {
                            pthread_cond_wait(&cond, &lock);
                        } 
                        available = 0;
                        kv_node_t *current = kv_node_head;
                        int key_found = 0;
                        while (current != NULL)
                        {
                            if (current->key == payload.key)
                            {
                                key_found = 1;
                                char *msg = "Key already exists";
                                send(client_fd, msg, strlen(msg), 0);
                                break;
                            }
                            current = current->next;
                        } 
                        if (key_found)
                        {
                            available = 1;
                            pthread_cond_signal(&cond);
                            pthread_mutex_unlock(&lock);
                            break;
                        } 
                        else
                        {
                            char *msg = "Key-value pair created";
                            send(client_fd, msg, strlen(msg), 0);
                        }
                        kv_node_t *new_node = (kv_node_t *)malloc(sizeof(kv_node_t));
                        if (kv_node_head == NULL)
                            kv_node_head = new_node;
                        else
                            kv_node_tail->next = new_node;
                        kv_node_tail = new_node;
                        new_node->next = NULL;
                        new_node->key = payload.key;
                        new_node->value = (char *)malloc(sizeof(char) * (payload.value_size + 1));
                        ssize_t value_bytes_read = recv(client_fd, new_node->value, payload.value_size + 1, 0);
                        if (value_bytes_read < 0)
                        {
                            perror("recv");
                        }
                        available = 1;
                        pthread_cond_signal(&cond);
                        pthread_mutex_unlock(&lock);
                        break;
                    }
                case CMD_READ:
                    {
                        pthread_mutex_lock(&lock);
                        while (available == 0) pthread_cond_wait(&cond, &lock);
                        available = 0;
                        kv_node_t *current = kv_node_head;
                        int key_found = 0;
                        while (current != NULL)
                        {
                            if (current->key == payload.key)
                            {
                                key_found = 1;
                                break;
                            }
                            current = current->next;
                        }
                        if (key_found == 0)
                        {
                            available = 1;
                            pthread_cond_signal(&cond);
                            pthread_mutex_unlock(&lock);
                            char *msg = "Key not found";
                            send(client_fd, msg, strlen(msg), 0);
                        }
                        else
                        {
                            send(client_fd, "OK", 2, 0);
                            int value_size = strlen(current->value);
                            memset(&payload, 0, sizeof(payload));
                            payload.value_size = value_size;
                            send(client_fd, &payload, sizeof(payload), 0);
                            send(client_fd, current->value, value_size, 0);
                            available = 1;
                            pthread_cond_signal(&cond);
                            pthread_mutex_unlock(&lock);
                        }
                        break;
                    }
                case CMD_DELETE:
                    {
                        pthread_mutex_lock(&lock);
                        while (available == 0) pthread_cond_wait(&cond, &lock);
                        available = 0;
                        kv_node_t *current = kv_node_head;
                        kv_node_t *previous = NULL;
                        int deleted = 0;
                        while (current != NULL)
                        {
                            if (current->key == payload.key)
                            {
                                if (current == kv_node_head)
                                {
                                    kv_node_head = current->next;
                                }
                                else
                            {
                                    previous->next = current->next;
                                }
                                free(current);
                                char* msg = "Key-value pair deleted";
                                send(client_fd, msg, strlen(msg), 0);
                                deleted = 1;
                                break;
                            }
                            previous = current;
                            current = current->next;
                        }
                        available = 1;
                        pthread_cond_signal(&cond);
                        pthread_mutex_unlock(&lock);
                        if (deleted == 0)
                            send(client_fd, "Key not found", 13, 0);
                        break;
                    }
                case CMD_UPDATE:
                    {
                        pthread_mutex_lock(&lock);
                        while (available == 0) pthread_cond_wait(&cond, &lock);
                        available = 0;
                        kv_node_t *current = kv_node_head;
                        int key_found = 0;
                        while (current != NULL)
                        {
                            if (current->key == payload.key)
                            {
                                key_found = 1;
                                break;
                            }
                        }
                        if (key_found == 0)
                        {
                            available = 1;
                            pthread_cond_signal(&cond);
                            pthread_mutex_unlock(&lock);
                            char *msg = "Key not found";
                            send(client_fd, msg, strlen(msg), 0);
                        }
                        else
                        {
                            char* msg = "Updated value";
                            send(client_fd, msg, strlen(msg), 0);
                            free(current->value);
                            current->value = (char *)malloc(sizeof(char) * (payload.value_size + 1));
                            ssize_t value_bytes_read = recv(client_fd, current->value, 1 + payload.value_size, 0);
                            if (value_bytes_read < 0)
                            {
                                perror("recv");
                            }
                            available = 1;
                            pthread_cond_signal(&cond);
                            pthread_mutex_unlock(&lock);
                        }
                        break;
                    }
                case CMD_DISCONNECT:
                    {
                        char *msg = "Disconnected from server";
                        send(client_fd, msg, strlen(msg), 0);
                        close(client_fd);
                        printf("Client disconnected\n");
                        client_disconnected = 1;
                        printf("Thread %d is free\n", thread_id);
                        break;
                    }
                default:
                    {
                        char *msg = "Invalid command";
                        send(client_fd, msg, strlen(msg), 0);
                        break;
                    }
            }
        }
        pthread_cond_signal(&fd_queue_cond);
    }
}

int main(int argc, char *argv[])
{

    struct hostent *server = gethostbyname(argv[1]);
    int portno = atoi(argv[2]);
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }
    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(sock_fd);
        return 1;
    }
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sock_fd);
        return 1;
    }
    if (listen(sock_fd, 5) < 0)
    {
        perror("listen");
        close(sock_fd);
        return 1;
    }
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    available = 1;
    pthread_mutex_init(&fd_queue_lock, NULL);
    pthread_cond_init(&fd_queue_cond, NULL);
    for (int i = 0; i < NO_OF_THREADS; i++) {
        thread_pool[i].is_busy = 0;
        thread_pool[i].thread_no = i;
        pthread_create(&thread_pool[i].tid, NULL, handle_client, &thread_pool[i].thread_no);
    }
    while (1)
    {
        printf("Waiting for client...\n");
        int client_fd = accept(sock_fd, NULL, NULL);
        fd_queue_t* new_fd_node = (fd_queue_t*) malloc(sizeof(fd_queue_t));
        new_fd_node->client_fd = client_fd;
        new_fd_node->next = NULL;
        pthread_mutex_lock(&fd_queue_lock);
        if (fd_queue_head == NULL) fd_queue_head = new_fd_node;
        else fd_queue_tail->next = new_fd_node;
        fd_queue_tail = new_fd_node;
        pthread_cond_signal(&fd_queue_cond);
        pthread_mutex_unlock(&fd_queue_lock);
    }
    for (int i = 0; i < NO_OF_THREADS; i++ ) pthread_join(thread_pool[i].tid, NULL);
    close(sock_fd);
    return 0;
}

