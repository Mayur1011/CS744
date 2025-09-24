/*
Write a program that takes two file names from the command line and copies the content
of the first file into the second file in reverse order (last character to first character) using
system calls like open, read, write, and close. Don’t use IO functions provided by stdio.h
like printf, fopen, fscanf, etc. You can assume that file size doesn’t exceed 500
characters.
*/

#include <fcntl.h>
#include <sys/types.h>

int main()
{
    char *input_file = "example.txt";
    char *output_file = "output4.txt";

    int input_fd = open(input_file, O_RDONLY);
    if (input_fd < 0)
    {
        perror("open input file");
        return 1;
    }

    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0)
    {
        perror("open output file");
        close(input_fd);
        return 1;
    }

    char buffer[500];
    ssize_t bytes_read = read(input_fd, buffer, sizeof(buffer));
    if (bytes_read < 0)
    {
        perror("read");
        close(input_fd);
        close(output_fd);
        return 1;
    }

    for (ssize_t i = bytes_read - 1; i >= 0; i--)
    {
        if (write(output_fd, &buffer[i], 1) != 1)
        {
            perror("write");
            close(input_fd);
            close(output_fd);
            return 1;
        }
    }

    close(input_fd);
    close(output_fd);
    return 0;
}