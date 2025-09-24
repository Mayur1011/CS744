#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_NAME "/tmp/my_scoket"
#define BUFFER_SIZE 1024

int main(void)
{

    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    unlink(SOCKET_NAME);

    struct sockaddr_un server_address;

    // bzero(&server_address, sizeof(struct sockaddr_un));
    memset(&server_address, 0, sizeof(struct sockaddr_un));

    server_address.sun_family = AF_UNIX;

    strncpy(server_address.sun_path, SOCKET_NAME, sizeof(server_address.sun_path) - 1);
    // strcpy(server_address.sun_path, SOCKET_NAME);

    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind");
        close(socket_fd);
        exit(1);
    }

    // Make this socket as passive socket
    if (listen(socket_fd, 5) < 0)
    {
        perror("listen");
        close(socket_fd);
        exit(1);
    }

    while (1)
    {
        printf("Waiting for a connection...\n");

        int client_fd = accept(socket_fd, NULL, NULL);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        printf("Client connected.\n");
    }

    return 0;
}
