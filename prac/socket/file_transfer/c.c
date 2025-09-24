#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#define SOCK_PATH "/tmp/file_transfer_socket"
#define BUFFER_SIZE 512
#define DATA_SIZE (BUFFER_SIZE - sizeof(int))

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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <remote_filename> <local_filename>\n", argv[0]);
        exit(1);
    }

    char *remote_filename = argv[1];
    char *local_filename = argv[2];

    int sockfd;
    struct sockaddr_un server_addr;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    // Send request
    char request[256];
    snprintf(request, sizeof(request), "SEND %s", remote_filename);

    if (sendto(sockfd, request, strlen(request), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    // Wait for response
    char response[256];
    ssize_t bytes_received = recvfrom(sockfd, response, sizeof(response) - 1, 0, NULL, NULL);

    if (bytes_received < 0)
    {
        perror("recvfrom");
        close(sockfd);
        exit(1);
    }

    response[bytes_received] = '\0';
    printf("Server response: %s\n", response);

    // Parse response
    char status[10], filename[100];
    long file_size;

    if (sscanf(response, "%s %s %ld", status, filename, &file_size) != 3 ||
        strcmp(status, "OK") != 0)
    {
        printf("Error: %s\n", response);
        close(sockfd);
        exit(1);
    }

    printf("Receiving file: %s (%ld bytes)\n", filename, file_size);

    // Create output file
    int file_fd = open(local_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0)
    {
        perror("create file");
        close(sockfd);
        exit(1);
    }

    // Receive file packets
    int expected_seq = 0;
    long total_received = 0;

    while (total_received < file_size)
    {
        packet_t packet;
        ssize_t packet_size = recvfrom(sockfd, &packet, sizeof(packet), 0,
                                       NULL, NULL);

        if (packet_size < 0)
        {
            perror("recvfrom packet");
            break;
        }

        // Check for EOF marker
        if (packet.seq_num == -1)
        {
            printf("Received EOF marker\n");
            break;
        }

        printf("Received packet %d (expected %d)\n", packet.seq_num, expected_seq);

        ack_t ack;
        ack.seq_num = packet.seq_num;

        if (packet.seq_num == expected_seq)
        {
            // Correct packet - write to file
            ssize_t data_size = packet_size - sizeof(int);
            if (write(file_fd, packet.data, data_size) != data_size)
            {
                perror("write to file");
                strcpy(ack.type, "NAK");
            }
            else
            {
                strcpy(ack.type, "ACK");
                total_received += data_size;
                expected_seq++;
                printf("Written %zd bytes to file\n", data_size);
            }
        }
        else
        {
            // Wrong sequence number
            printf("Wrong sequence number. Expected %d, got %d\n",
                   expected_seq, packet.seq_num);
            strcpy(ack.type, "NAK");
        }

        // Send acknowledgment
        sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    close(file_fd);
    close(sockfd);

    printf("File transfer completed. Total bytes received: %ld\n", total_received);
    return 0;
}