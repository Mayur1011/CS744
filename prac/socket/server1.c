#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCK_ADDR "/tmp/mysocket"
#define BUF_SIZE 100

int main(void)
{

    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    unlink(SOCK_ADDR);

    struct sockaddr_un server_addr = {0};
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_ADDR);

    // strncpy(server_addr.sun_path, SOCK_ADDR, sizeof(server_addr.sun_path) - 1);
    // server_addr.sun_path[sizeof(server_addr.sun_path) - 1] = '\0';

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind");
        close(socket_fd);
    }

    char buff[BUF_SIZE] = {0};

    // char *buff;
    // buff = (char *)malloc(BUF_SIZE * sizeof(char));
    // if (buff == NULL)
    // {
    //     perror("malloc");
    //     close(socket_fd);
    //     return EXIT_FAILURE;
    // }

    struct sockaddr_un client_addr = {0};
    socklen_t client_addr_len = sizeof(struct sockaddr_un);

    ssize_t num_bytes = recvfrom(socket_fd, buff, BUF_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);

    if (num_bytes == -1)
    {
        perror("recvfrom");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    buff[num_bytes] = '\0';

    printf("Received %zd bytes: %s\n", num_bytes, buff);

    close(socket_fd);
    unlink(SOCK_ADDR);
    free(buff);
    return EXIT_SUCCESS;
}
