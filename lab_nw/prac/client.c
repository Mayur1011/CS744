#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

int main (int argc, char* argv[]) {

    struct hostent* server = gethostbyname(argv[1]);
    int portno = atoi(argv[2]);
    // char* server_ip = argv[1];
    
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);
    // if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    // {
    //     printf("Invalid address / Address not supported\n");
    //     return -1;
    // }
    server_addr.sin_port = htons(portno);
    if( connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ){
        perror("connect");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    char buffer[1024];
    while (1) {
        bzero((char*)&buffer, sizeof(buffer));
        printf("Enter message: ");
        fgets(buffer, sizeof(buffer) - 1, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        ssize_t bytes_written = write(socket_fd, buffer, strlen(buffer));
        if (bytes_written < 0) { perror("write"); }

        bzero((char*)&buffer, sizeof(buffer));
        ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) { perror("read"); }
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);

        if (strncmp("exit", buffer, 4) == 0) {
            printf("Exiting...\n");
            break;
        }
    }
    close(socket_fd);
}
