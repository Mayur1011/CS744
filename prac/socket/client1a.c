#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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

    struct sockaddr_un server_addr = {0};
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_ADDR);

    char buf[BUF_SIZE];
    fgets(buf, BUF_SIZE, stdin);

    size_t buf_len = strlen(buf);
    if (buf_len > 0 && buf[buf_len - 1] == '\n')
        buf[buf_len - 1] = '\0';
    printf("Sending: %s\n", buf);

    ssize_t num_bytes = sendto(socket_fd, buf, buf_len, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));

    return EXIT_SUCCESS;
}