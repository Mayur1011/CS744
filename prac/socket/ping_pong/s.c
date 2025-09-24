#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/errno.h>

#define SOCKET_PATH "/tmp/health.sock"
#define BUFFER_SIZE 32

int main()
{
    int server_sock;
    struct sockaddr_un server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len;

    // 1. Create a datagram Unix domain socket.
    server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Set up the server_addr structure.
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. Unlink the socket path to ensure it doesn't already exist.
    // This prevents "Address already in use" errors on restart.
    if (unlink(SOCKET_PATH) == -1 && errno != ENOENT)
    {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    // 4. Bind the socket to the server address.
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Health server is listening on %s\n", SOCKET_PATH);
    printf("Waiting for PING requests...\n");

    while (1)
    {
        client_addr_len = sizeof(struct sockaddr_un);
        // 5. Wait to receive a message.
        ssize_t bytes_recvd = recvfrom(server_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);

        printf("Received %zd bytes from client.\n", bytes_recvd);
        printf("Client addr len: %d\n", client_addr_len);
        fflush(stdout);

        if (bytes_recvd == -1)
        {
            perror("recvfrom");
            continue; // Continue to the next iteration on error
        }

        // Null-terminate the received data to treat it as a string
        buffer[bytes_recvd] = '\0';
        printf("Received message: \"%s\"\n", buffer);

        // 6. Check the message and respond accordingly.
        if (strcmp(buffer, "PING") == 0)
        {
            printf("  -> Valid PING. Responding with PONG after 1 second.\n");
            sleep(1); // Introduce a delay
            if (sendto(server_sock, "PONG", 4, 0, (struct sockaddr *)&client_addr, client_addr_len) == -1)
            {
                perror("sendto PONG");
            }
        }
        else
        {
            printf("  -> Invalid message. Responding with ERROR.\n");
            if (sendto(server_sock, "ERROR", 5, 0, (struct sockaddr *)&client_addr, client_addr_len) == -1)
            {
                perror("sendto ERROR");
            }
        }
    }

    // This part is unreachable in this simple design but good practice.
    close(server_sock);
    unlink(SOCKET_PATH);

    return 0;
}