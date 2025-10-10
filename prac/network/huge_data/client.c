#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 6969
#define ADDR "localhost"

typedef struct __payload_t {
    int size;
} payload_t;

int main () {
    struct hostent *server = gethostbyname(ADDR);
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    // memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    bcopy(server->h_addr_list[0], &server_addr.sin_addr.s_addr, server->h_length);
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("scoket");
        exit(EXIT_FAILURE);
    }
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    payload_t payload;
    scanf("%d", &payload.size);
    char* buff = (char*) malloc(payload.size + 1);
    memset(buff, 0, strlen(buff));
    int total_bytes = 0;
    char tempbuff[512];
    ssize_t bytes_read = send(sock_fd, &payload, sizeof(payload), 0);
    while((bytes_read = read(0, tempbuff, sizeof(tempbuff) - 1)) > 0) {
        tempbuff[bytes_read] = '\0';
        strcat(buff, tempbuff);
        int bytes_sent = send(sock_fd, tempbuff, sizeof(tempbuff), 0);
        if (bytes_sent < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        bzero(tempbuff, sizeof(tempbuff));
        total_bytes += bytes_read;
        if (total_bytes >= payload.size) {
            buff[payload.size] = '\0';
            break;
        }
    }
    printf("%s\n", buff);
    close(sock_fd);
    free(buff);
}
