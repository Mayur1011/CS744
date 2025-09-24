#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int sv[2];

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0)
    {
        perror("socketpair");
        exit(1);
    }

    int pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        close(sv[0]);

        char msg[100] = {0};

        ssize_t bytes_recv = recv(sv[1], (void *)msg, sizeof(msg) - 1, 0);
        if (bytes_recv < 0)
        {
            perror("recv");
            exit(1);
        }

        msg[bytes_recv] = '\0';

        printf("Child received: %s\n", msg);

        close(sv[1]);
        exit(0);
    }
    else
    {
        close(sv[1]);

        const char *msg = "Hello from parent";
        ssize_t bytes_sent = send(sv[0], msg, 17, 0);

        if (bytes_sent < 0)
        {
            perror("send");
            exit(1);
        }

        close(sv[0]);
        wait(NULL);
    }
}
