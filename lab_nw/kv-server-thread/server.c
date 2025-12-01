#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include <pthread.h>

typedef struct __kv_node
{
    int key;
    char *value;
    struct __kv_node *next;
} kv_node_t;

pthread_mutex_t lock;
pthread_cond_t cond;
int available = 1;
kv_node_t *head = NULL;
kv_node_t *tail = NULL;

void *handle_client(void *arg)
{
    int client_fd = *((int *)arg);
    while (1)
    {
        payload_t payload;
        bzero(&payload, sizeof(payload));
        ssize_t bytes_read = recv(client_fd, &payload, sizeof(payload), 0);
        if (bytes_read < 0)
        {
            perror("recv");
        }

        // printf("Command type: %d\n", payload.cmd_type);
        // printf("Key: %d\n", payload.key);
        // printf("Value size: %d\n", payload.value_size);

        switch (payload.cmd_type)
        {
        case CMD_CREATE:
        {
            pthread_mutex_lock(&lock);
            while (available == 0)
            {
                pthread_cond_wait(&cond, &lock);
            }
            available = 0;
            kv_node_t *current = head;
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
            if (head == NULL)
                head = new_node;
            else
                tail->next = new_node;
            tail = new_node;
            new_node->next = NULL;
            new_node->key = payload.key;
            new_node->value = (char *)malloc(sizeof(char) * (payload.value_size + 1));
            // printf("<HERE\n");
            ssize_t value_bytes_read = recv(client_fd, new_node->value, payload.value_size + 1, 0);
            // printf("value bytes read: %zd\n", value_bytes_read);
            // printf("value size: %d\n", payload.value_size);
            // printf("Value received: %s\n", new_node->value);
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
            // printf("Key to read: %d\n", payload.key);
            pthread_mutex_lock(&lock);
            while (available == 0)
                pthread_cond_wait(&cond, &lock);
            available = 0;
            kv_node_t *current = head;
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
                // printf("Payload value size : %d\n", payload.value_size);
                // printf("Value: %s\n", current->value);
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
            while (available == 0)
                pthread_cond_wait(&cond, &lock);
            available = 0;
            kv_node_t *current = head;
            kv_node_t *previous = NULL;
            int deleted = 0;
            while (current != NULL)
            {
                if (current->key == payload.key)
                {
                    if (current == head)
                    {
                        head = current->next;
                    }
                    else
                    {
                        previous->next = current->next;
                    }
                    free(current);
                    char *msg = "Key-value pair deleted";
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
            while (available == 0)
                pthread_cond_wait(&cond, &lock);
            available = 0;
            kv_node_t *current = head;
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
                char *msg = "Updated value";
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

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

    while (1)
    {
        printf("Waiting...\n");
        int client_fd = accept(sock_fd, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &client_fd);
        printf("Client connected with client_fd %d and thread_id %lu\n", client_fd, tid);
    }
    close(sock_fd);
    return 0;
}
