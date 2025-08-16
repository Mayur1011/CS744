#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

const int CAT_BUFF_SIZE = 10;

int main(int argc, char** argv) {
    char buf[CAT_BUFF_SIZE];
    size_t bytes_read;
    int buf_idx = 0;
    char tempchar;
    if (argc == 1) {
        while(1) {
            while(buf_idx < CAT_BUFF_SIZE) {
                bytes_read = read(0, &tempchar, 1);
                buf[buf_idx] = tempchar;
                if(tempchar == '\n') {
                    write(1, buf, buf_idx + 1);
                    buf_idx = 0;
                    break;
                }
                buf_idx += 1;
                if(buf_idx == CAT_BUFF_SIZE || bytes_read == 0) {
                    write(1, buf, buf_idx + 1);
                    buf_idx = 0;
                }
            }
        }
    }
    int open_fd = open(argv[1], O_RDONLY);
    if (open_fd == -1) {
        perror("Error while opening the file");
        exit(EXIT_FAILURE);
    }

    while((bytes_read = read(open_fd, buf, sizeof(buf))) > 0) {
        size_t bytes_written = write(1, buf, bytes_read);
        if (bytes_read != bytes_written) {
            perror("write error");
            close(open_fd);
            exit(EXIT_FAILURE);
        }
    } 

    close(open_fd);
}
