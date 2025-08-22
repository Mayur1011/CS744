#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {

    int file_fd = open("./foo.txt", O_RDONLY);

    char buf[256];
    int read_bytes = 0;
    while ((read_bytes = read(file_fd, buf, sizeof(buf))) > 0) {
        if (read_bytes <= 0) {
            perror("read");
            close(file_fd);
            return 1;
        }
        write(STDOUT_FILENO, buf, read_bytes);
    }
    return 0;
}
