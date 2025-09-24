/*
Write a program that takes three filenames and an integer offset as argument, copies the content of the first file to the third file, and then additionally copies the content of the
second file to the third file starting at the mentioned offset.
The catch is that you should not use the file and IO-related functions provided via stdio.h
(e.g., scanf, printf, fopen, fread, fwrite, fprintf, fscanf, …).
*/

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        write(2, "Usage: ./a.out <file1> <file2> <file3> <offset>\n", 48);
        return 1;
    }

    int offset = atoi(argv[4]);

    int fd1 = open(argv[1], O_RDONLY);
    int fd2 = open(argv[2], O_RDONLY);
    int fd3 = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd1 == -1 || fd2 == -1 || fd3 == -1)
    {
        write(2, "Error opening files\n", 20);
        return 1;
    }

    char buffer[1024];
    ssize_t bytesRead;

    // Copy content from file1 to file3
    while ((bytesRead = read(fd1, buffer, sizeof(buffer))) > 0)
    {
        write(fd3, buffer, bytesRead);
    }

    // Copy content from file2 to file3 at the specified offset
    lseek(fd3, offset, SEEK_SET);
    while ((bytesRead = read(fd2, buffer, sizeof(buffer))) > 0)
    {
        write(fd3, buffer, bytesRead);
    }

    close(fd1);
    close(fd2);
    close(fd3);

    return 0;
}