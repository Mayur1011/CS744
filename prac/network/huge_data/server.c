#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 6969

typedef struct __payload_t {
    int size;
} payload_t;

int main() {
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {perror("socket"); exit(EXIT_FAILURE);}
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { perror("setscokopt"); exit(EXIT_FAILURE); }
    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { perror("bind"); exit(EXIT_FAILURE); }
    if (listen(sock_fd, 5) < 0) { perror("listen"); exit(EXIT_FAILURE); }
    while (1) {
        int client_fd = accept(sock_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        payload_t payload;
        ssize_t bytes_recieved = recv(client_fd, &payload, sizeof(payload), 0);
        char* buffer = (char*) malloc(payload.size);
        char temp_buffer[5] = {0};
        bytes_recieved = 0;
        int total_bytes_recieved = 0;
        while((bytes_recieved = recv(client_fd, temp_buffer, 4, 0)) > 0) {
            temp_buffer[bytes_recieved] = '\0';
            strcat(buffer, temp_buffer);
            total_bytes_recieved += bytes_recieved;
            if (total_bytes_recieved >= payload.size) { break; }
            bzero(temp_buffer, sizeof(temp_buffer));
        }
        printf("Final string is : %s\n", buffer);
        close(client_fd);
    }
    close(sock_fd);
    return 0;
}
