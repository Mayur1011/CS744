#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>


#define SOCK_ADDR "/tmp/mysocket"
const int BUFFER_SIZE = 10;

int main() {

    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        exit(1);
    }

    unlink(SOCK_ADDR);

    struct sockaddr_un server_addr;

    bzero(&server_addr, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_ADDR);

    if(bind(socket_fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    struct sockaddr_un client_addr;

    while(1) {
        bzero(buffer, BUFFER_SIZE);
        socklen_t client_len = sizeof(struct sockaddr_un);
        ssize_t n = recvfrom(socket_fd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*) &client_addr, &client_len);
        if (n < 0) {
            perror("recvfrom");
            exit(1);
        }
        printf("First : %s\n", buffer);
    }
}
