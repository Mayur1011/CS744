#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "types.h"

int main()
{

    while (1)
    {
        printf("Enter a comamnd: ");
        char *cmd = (char *)malloc(256 * sizeof(char));
        scanf("%s", cmd);

        int sock_fd;
        int is_connected = 0;
        struct sockaddr_in server_addr;
        int portno;

        if (strcmp(cmd, "connect") == 0)
        {
            char *server_name = (char *)malloc(256);
            scanf("%s", server_name);
            scanf("%d", &portno);
            if (is_connected)
            {
                printf("Already connected to server\n");
                return 1;
            }
            // printf("%s, %d", server_name, portno);
            struct hostent *server = gethostbyname(server_name);
            sock_fd = socket(AF_INET, SOCK_STREAM, 0);
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(portno);
            bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
            if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                perror("connect");
                close(sock_fd);
                return 1;
            }
            is_connected = 1;
            printf("Connected to server %s at port %d\n", server_name, portno);
        }
        else if (strcmp(cmd, "disconnect") == 0)
        {
            payload_t payload;
            payload.cmd_type = CMD_DISCONNECT;
            ssize_t bytes_written = send(sock_fd, &payload, sizeof(payload), 0);
            if (bytes_written < 0)
            {
                perror("send");
            }
            char buffer[256];
            ssize_t bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }
            buffer[bytes_read] = '\0';
            printf("Server response: %s\n", buffer);
            is_connected = 0;
            close(sock_fd);
        }
        else if (strcmp(cmd, "create") == 0)
        {
            payload_t payload;
            payload.cmd_type = CMD_CREATE;
            scanf("%d %d", &payload.key, &payload.value_size);
            ssize_t bytes_written = send(sock_fd, &payload, sizeof(payload), 0);
            if (bytes_written < 0)
            {
                perror("Send");
                return 1;
            }
            getchar();
            char *value = (char *)malloc(sizeof(char) * (1 + payload.value_size));
            fgets(value, 1 + payload.value_size, stdin);
            // printf("Value to send: %s\n", value);
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }
            buffer[bytes_read] = '\0';
            if (strcmp(buffer, "OK") != 0)
            {
                printf("Server response: %s\n", buffer);
                free(value);
                continue;
            }
            bytes_written = send(sock_fd, value, strlen(value), 0);
            if (bytes_written < 0)
            {
                perror("Send");
                return 1;
            }
            printf("Server response: %s\n", buffer);
        }
        else if (strcmp(cmd, "read") == 0)
        {
            payload_t payload;
            payload.cmd_type = CMD_READ;
            scanf("%d", &payload.key);
            ssize_t bytes_written = send(sock_fd, &payload, sizeof(payload), 0);
            if (bytes_written < 0)
            {
                perror("send");
            }
            char response[256];
            ssize_t bytes_read = recv(sock_fd, response, sizeof(response) - 1, 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }
            response[bytes_read] = '\0';
            if (strcmp(response, "OK") != 0)
            {
                continue;
            }
            memset(&payload, 0, sizeof(payload));
            ssize_t size_read = recv(sock_fd, &payload, sizeof(payload), 0);
            if (size_read < 0)
            {
                perror("recv");
            }
            // printf("Value size: %d\n", payload.value_size);
            char *buffer = (char *)malloc(sizeof(char) * (payload.value_size + 1));
            bytes_read = recv(sock_fd, buffer, payload.value_size, 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }
            buffer[bytes_read] = '\0';
            printf("Value: %s\n", buffer);
            free(buffer);
        }
        else if (strcmp(cmd, "update") == 0)
        {
            payload_t payload;
            payload.cmd_type = CMD_UPDATE;
            scanf("%d %d", &payload.key, &payload.value_size);
            ssize_t bytes_written = send(sock_fd, &payload, sizeof(payload), 0);
            if (bytes_written < 0)
            {
                perror("send");
            };
            getchar();
            char *value = (char *)malloc(sizeof(char) * (1 + payload.value_size));
            fgets(value, 1 + payload.value_size, stdin);

            char response[256];
            ssize_t bytes_read = recv(sock_fd, response, sizeof(response) - 1, 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }
            response[bytes_read] = '\0';
            printf("Server response: %s\n", response);
            if (strcmp(response, "OK") != 0)
            {
                continue;
            }

            bytes_written = send(sock_fd, value, strlen(value), 0);
            if (bytes_written < 0)
            {
                perror("send");
            }
            free(value);
        }
        else if (strcmp(cmd, "delete") == 0)
        {
            payload_t payload;
            payload.cmd_type = CMD_DELETE;
            scanf("%d", &payload.key);
            ssize_t bytes_written = send(sock_fd, &payload, sizeof(payload), 0);
            if (bytes_written < 0)
            {
                perror("send");
            }
            char response[256];
            ssize_t bytes_read = recv(sock_fd, response, sizeof(response) - 1, 0);
            if (bytes_read < 0)
            {
                perror("recv");
            }
            response[bytes_read] = '\0';
            printf("Server response: %s\n", response);
        }
        else
        {
            printf("HERE Invalid command\n");
        }
        free(cmd);
    }
    return 0;
}
