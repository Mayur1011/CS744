#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SOCK_ADDR "/tmp/mysocket"
const int BUFFER_SIZE = 1024;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s File name not specified\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;

    bzero(&server_addr, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_ADDR);

    int file_fd = open(argv[1], O_RDONLY);
    if (file_fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Send the server size of the file using fstat
    struct stat sb;
    if (fstat(file_fd, &sb) == -1)
    {
        perror("fstat");
        close(file_fd);
        close(socket_fd);
        return 1;
    }

    int64_t file_size = sb.st_size;
    ssize_t sent_bytes = sendto(socket_fd, &file_size, sizeof(file_size), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
    if (sent_bytes < 0)
    {
        perror("sendto");
        close(file_fd);
        close(socket_fd);
        return 1;
    }

    // Read from file and send to server
    char buf[BUFFER_SIZE];
    int read_bytes = 0;
    while ((read_bytes = read(file_fd, buf, sizeof(buf))) > 0)
    {
        if (read_bytes <= 0)
        {
            perror("read");
            close(file_fd);
            close(socket_fd);
            return 1;
        }
        // write(STDOUT_FILENO, buf, read_bytes);
        ssize_t sent_bytes = sendto(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
        if (sent_bytes < 0)
        {
            perror("sendto");
            close(file_fd);
            close(socket_fd);
            return 1;
        }
    }

    close(file_fd);
    close(socket_fd);
    return 0;
}
