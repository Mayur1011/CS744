/*
Implement file transfer using multiple datagrams:

Protocol: SEND filename size
Then send file in chunks with sequence numbers
Handle acknowledgments and retransmissions
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define SOCK_PATH "/tmp/file_transfer_socket"
#define BUFFER_SIZE 512
#define DATA_SIZE (BUFFER_SIZE - sizeof(int)) // Space for sequence number
#define MAX_RETRIES 3
#define TIMEOUT_SEC 2

typedef struct
{
    int seq_num;
    char data[DATA_SIZE];
} packet_t;

typedef struct
{
    int seq_num;
    char type[4]; // "ACK" or "NAK"
} ack_t;

int main()
{
    int sockfd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // Remove existing socket file
    unlink(SOCK_PATH);

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("File transfer server started. Waiting for requests...\n");

    while (1)
    {
        char request[256];
        ssize_t bytes_received = recvfrom(sockfd, request, sizeof(request) - 1, 0, (struct sockaddr *)&client_addr, &client_len);

        if (bytes_received < 0)
        {
            perror("recvfrom");
            continue;
        }

        request[bytes_received] = '\0';
        printf("Received request: %s\n", request);

        // Parse request: "SEND filename"
        char command[10], filename[100];
        if (sscanf(request, "%s %s", command, filename) != 2 || strcmp(command, "SEND") != 0)
        {
            char error_msg[] = "ERROR: Invalid request format. Use: SEND filename";
            sendto(sockfd, error_msg, strlen(error_msg), 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

        // Check if file exists and get size
        struct stat file_stat;
        if (stat(filename, &file_stat) < 0)
        {
            char error_msg[] = "ERROR: File not found";
            sendto(sockfd, error_msg, strlen(error_msg), 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

        // Send file info: "OK filename size"
        char response[256];
        snprintf(response, sizeof(response), "OK %s %ld", filename, file_stat.st_size);
        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_len);

        // Open file for reading
        int file_fd = open(filename, O_RDONLY);
        if (file_fd < 0)
        {
            perror("open file");
            continue;
        }

        // Send file in chunks
        packet_t packet;
        int seq_num = 0;
        long total_sent = 0;

        while (total_sent < file_stat.st_size)
        {
            // Read chunk from file
            ssize_t bytes_read = read(file_fd, packet.data, DATA_SIZE);
            if (bytes_read <= 0)
                break;

            packet.seq_num = seq_num;

            // Send packet with retries
            int retry_count = 0;
            int ack_received = 0;

            while (retry_count < MAX_RETRIES && !ack_received)
            {
                // Send packet
                sendto(sockfd, &packet, sizeof(int) + bytes_read, 0, (struct sockaddr *)&client_addr, client_len);

                printf("Sent packet %d (%zd bytes)\n", seq_num, bytes_read);

                // Wait for ACK with timeout
                fd_set readfds;
                struct timeval timeout;

                FD_ZERO(&readfds);
                FD_SET(sockfd, &readfds);
                timeout.tv_sec = TIMEOUT_SEC;
                timeout.tv_usec = 0;

                int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

                if (select_result > 0)
                {
                    ack_t ack;
                    struct sockaddr_un ack_addr;
                    socklen_t ack_len = sizeof(ack_addr);

                    ssize_t ack_bytes = recvfrom(sockfd, &ack, sizeof(ack), 0,
                                                 (struct sockaddr *)&ack_addr, &ack_len);

                    if (ack_bytes > 0 && ack.seq_num == seq_num && strcmp(ack.type, "ACK") == 0)
                    {
                        ack_received = 1;
                        printf("Received ACK for packet %d\n", seq_num);
                    }
                }
                else if (select_result == 0)
                {
                    printf("Timeout waiting for ACK %d, retrying...\n", seq_num);
                    retry_count++;
                }
                else
                {
                    perror("select");
                    break;
                }
            }

            if (!ack_received)
            {
                printf("Failed to send packet %d after %d retries\n", seq_num, MAX_RETRIES);
                break;
            }

            total_sent += bytes_read;
            seq_num++;
        }

        close(file_fd);

        // Send end-of-file marker
        packet.seq_num = -1; // Special sequence number for EOF
        strcpy(packet.data, "EOF");
        sendto(sockfd, &packet, sizeof(int) + 3, 0,
               (struct sockaddr *)&client_addr, client_len);

        printf("File transfer completed. Total bytes sent: %ld\n", total_sent);
    }

    close(sockfd);
    unlink(SOCK_PATH);
    return 0;
}