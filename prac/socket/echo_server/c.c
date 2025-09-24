#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCK_SERVER_ADDR "/tmp/socket"
#define SOCK_CLIENT_ADDR "/tmp/client"

int main()
{
    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un client_address = {0};
    client_address.sun_family = AF_UNIX;
    strcpy(client_address.sun_path, SOCK_CLIENT_ADDR);

    unlink(client_address.sun_path);

    if (bind(socket_fd, (struct sockaddr *)&client_address, sizeof(client_address)) == -1)
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    struct sockaddr_un server_address = {0};
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SOCK_SERVER_ADDR);

    // const char *message = "Hello from client";

    char message[100];
    snprintf(message, sizeof(message), "Hello from client");

    ssize_t num_bytes = sendto(
        socket_fd,
        message,
        strlen(message),
        0,
        (struct sockaddr *)&server_address,
        sizeof(server_address));

    if (num_bytes == -1)
    {
        perror("sendto");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    char buffer[100] = {0};

    num_bytes = recvfrom(
        socket_fd,
        buffer,
        sizeof(buffer) - 1,
        0,
        NULL,
        NULL);

    if (num_bytes == -1)
    {
        perror("recvfrom");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    write(STDOUT_FILENO, buffer, num_bytes);

    close(socket_fd);
    return EXIT_SUCCESS;
}
