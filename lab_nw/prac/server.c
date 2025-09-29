#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char* argv[]) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    printf("Socket created successfully with file descriptor: %d\n", socket_fd);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6969);

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    printf("Socket bound to port 6969 successfully.\n");

    listen(socket_fd, 5);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    char buffer[1024];
    while(1){
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        // ? Does read automatically null-terminate the string?
        if (bytes_read < 0) { perror("read"); }
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);

        bzero((char*)&buffer, sizeof(buffer));
        printf("Enter message: ");
        fgets(buffer, sizeof(buffer) - 1, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        ssize_t bytes_written = write(client_fd, buffer, strlen(buffer));
        if (bytes_written < 0) { perror("write"); }

        if (strncmp("exit", buffer, 4) == 0) {
            printf("Exiting...\n");
            break;
        }
    }
    close(client_fd);
    close(socket_fd);
    return 0;
}
