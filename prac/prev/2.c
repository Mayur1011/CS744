/*
Create a parent process that redirects standard output to a file using dup2().
Fork a child process.
Investigate whether the file descriptor redirection is inherited by the child process as well.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int file_fd = open("example.txt", O_RDWR | O_CREAT, 0644);
    if (file_fd < 0)
    {
        perror("open");
        return 1;
    }

    dup2(file_fd, STDOUT_FILENO);

    pid_t child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (child_pid == 0)
    {
        write(STDOUT_FILENO, "Hello from child process!\n", 26);
        exit(0);
    }
    else
        write(STDOUT_FILENO, "Hello, World!\n", 14);

    close(file_fd);
}