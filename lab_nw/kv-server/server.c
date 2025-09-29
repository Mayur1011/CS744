#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "types.h"

typedef struct __kv_node {
    int key;
    char* value;
    struct __kv_node* next;
} kv_node_t;


int main (int argc, char* argv[]) {

    struct hostent* server = gethostbyname(argv[1]);
    int portno = atoi(argv[2]);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);

    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sock_fd);
        return 1;
    }

    if(bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0){
        perror("bind");
        close(sock_fd);
        return 1;
    }

    if (listen(sock_fd, 5) < 0) {
        perror("listen");
        close(sock_fd);
        return 1;
    }

    char buffer[10];
    kv_node_t* head = NULL;
    kv_node_t* tail = NULL;
    int client_present = 0;
    while(1) {

        int client_fd = accept(sock_fd, NULL, NULL);

        if (client_present){
            char* msg = "Another client is already connected";
            send(client_fd, msg, strlen(msg), 0);
            close(client_fd);
            continue;
        }

        client_present = 1;

        printf("Client connected\n");
        payload_t payload;
        while (1) {
            if (client_present == 0) break;
            bzero(&payload, sizeof(payload));
            ssize_t bytes_read = read(client_fd, &payload, sizeof(payload));
            if (bytes_read < 0) { perror("read"); }

            // printf("Command type: %d\n", payload.cmd_type);
            // printf("Key: %d\n", payload.key);
            // printf("Value size: %d\n", payload.value_size);

            switch (payload.cmd_type) {
                case CMD_CREATE: {
                    kv_node_t* current = head;
                    int key_found = 0;
                    while (current != NULL) {
                        if (current->key == payload.key) {
                            key_found = 1;
                            char* msg = "Key already exists";
                            send(client_fd, msg, strlen(msg), 0);
                            break;
                        }
                        current = current->next;
                    }
                    if (key_found) continue;
                    kv_node_t* new_node = (kv_node_t*) malloc(sizeof(kv_node_t));
                    if (head == NULL) {
                        head = new_node;
                    } else {
                        tail->next = new_node;
                    }
                    tail = new_node;
                    new_node->next = NULL;
                    new_node->key = payload.key;
                    new_node->value = (char*) malloc(sizeof(char) * (payload.value_size + 1));
                    ssize_t value_bytes_read = read(client_fd, new_node->value, 1 + payload.value_size);
                    if (value_bytes_read < 0) { perror("read"); }
                    // new_node->value[payload.value_size] = '\0';
                    ssize_t bytes_send = send(client_fd, "OK", 2, 0);
                    if(bytes_send < 0) { perror("send"); }
                    break;
                }
                case CMD_READ: {
                    // printf("Key to read: %d\n", payload.key);
                    kv_node_t* current = head;
                    int key_found = 0;
                    while (current != NULL) {
                        if (current->key == payload.key) {
                            key_found = 1;
                            int value_size = strlen(current->value);
                            send(client_fd, &value_size, sizeof(int), 0);
                            send(client_fd, current->value, value_size, 0);
                            break;
                        }
                        current = current->next;
                    }
                    if (key_found == 0) {
                        char* msg = "Key not found";
                        send(client_fd, msg, strlen(msg), 0);
                    }
                    break;
                }
                case CMD_DELETE: {
                }
                case CMD_UPDATE: {
                    kv_node_t* current = head;
                    int key_found = 0;
                    while(current != NULL) {
                        if (current->key == payload.key) {
                            key_found = 1;
                            free(current->value);
                            current->value = (char*) malloc(sizeof(char) * (payload.value_size + 1));
                            ssize_t value_bytes_read = read(client_fd, current->value, 1 + payload.value_size);
                            if (value_bytes_read < 0) { perror("read"); }
                            ssize_t bytes_send = send(client_fd, "OK", 2, 0);
                            if (bytes_send < 0) { perror("send"); }
                            break;
                        }
                    }
                    if (key_found == 0) {
                        char* msg = "Key not found";
                        send(client_fd, msg, strlen(msg), 0);
                    }
                    break;
                }
                case CMD_DISCONNECT: {
                    char* msg = "Disconnected from server";
                    send(client_fd, msg, strlen(msg), 0);
                    close(client_fd);
                    client_present = 0;
                    printf("Client disconnected\n");
                    break;
                }
                default: {
                    char* msg = "Invalid command";
                    send(client_fd, msg, strlen(msg), 0);
                    break;
                }
            }
        }
    }
    close(sock_fd);
    return 0;
}
