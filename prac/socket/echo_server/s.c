#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>

#define SOCK_ADDR "/tmp/socket"

int main(void)
{

    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (socket_fd < 0)
    {
        perror("socket");
        return -1;
    }

    unlink(SOCK_ADDR);

    struct sockaddr_un server_address = {0};
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SOCK_ADDR);

    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    char buffer[100] = {0};
    struct sockaddr_un client_address = {0};
    socklen_t client_address_len = sizeof(client_address);
    printf("Waiting for client data...\n");

    ssize_t num_bytes = recvfrom(socket_fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_address, &client_address_len);

    if (num_bytes < 0)
    {
        perror("recvfrom");
        return EXIT_FAILURE;
    }

    buffer[num_bytes] = '\0';

    write(STDOUT_FILENO, buffer, num_bytes);

    // send back to client
    num_bytes = sendto(socket_fd, buffer, num_bytes, 0, (struct sockaddr *)&client_address, client_address_len);

    if (num_bytes < 0)
    {
        perror("sendto");
        return EXIT_FAILURE;
    }

    close(socket_fd);

    return 0;
}
