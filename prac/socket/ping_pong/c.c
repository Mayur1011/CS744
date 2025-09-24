#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>

#define SERVER_PATH "/tmp/health.sock"
#define CLIENT_PATH "/tmp/health_c.sock"
#define BUFFER_SIZE 32

int main(int argc, char *argv[])
{
    int timeout_ms = 2000; // Default timeout of 2 seconds
    if (argc > 1)
    {
        timeout_ms = atoi(argv[1]);
        if (timeout_ms <= 0)
        {
            fprintf(stderr, "Timeout must be a positive integer.\n");
            exit(EXIT_FAILURE);
        }
    }

    int sock;
    struct sockaddr_un server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int exit_code = 1; // Default to failure

    // 1. Create a datagram Unix socket.
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Set the receive timeout on the socket.
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
    {
        perror("setsockopt");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 3. Set up the server_addr structure.
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. Set up the client_addr structure.
    memset(&client_addr, 0, sizeof(struct sockaddr_un));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, CLIENT_PATH, sizeof(client_addr.sun_path) - 1);

    if (unlink(CLIENT_PATH) < 0 && errno != ENOENT)
    {
        perror("unlink");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_un)) < 0)
    {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 4. Send the "PING" message to the server.
    printf("Pinging server with a %dms timeout...\n", timeout_ms);
    if (sendto(sock, "PING", 4, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("sendto");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 5. Call recvfrom() to wait for the response.
    ssize_t bytes_recvd = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);

    // 6. Check the return value and errno to determine the outcome.
    if (bytes_recvd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // This is the expected error for a timeout
            fprintf(stderr, "FAILURE: Server did not respond in time.\n");
        }
        else
        {
            // Any other error
            perror("recvfrom");
        }
    }
    else
    {
        // Data was received, null-terminate it.
        buffer[bytes_recvd] = '\0';

        // Check if the response is what we expect.
        if (strcmp(buffer, "PONG") == 0)
        {
            printf("SUCCESS: Server is alive.\n");
            exit_code = 0; // Success
        }
        else
        {
            fprintf(stderr, "FAILURE: Server sent an unexpected response: \"%s\".\n", buffer);
        }
    }

    close(sock);
    return exit_code;
}