#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    int pipe_fd[2]; // These file descriptors are shared between parent-child processes.
    pipe(pipe_fd);
    pid_t fork_result = fork();
    if (fork_result == 0)
    {
        close(pipe_fd[1]);
        char read_buf[1000];
        if (read(pipe_fd[0], read_buf, sizeof(read_buf)) == -1)
        {
            perror("Read from pipe failed");
        }
        int i = 0;
        while (read_buf[i])
        {
            printf("%c", read_buf[i]);
            i++;
        }
        close(pipe_fd[0]);
    }
    else
    {
        // write to the pipe
        close(pipe_fd[0]);
        char write_buf[1000] = "This write is from parent\n";
        if (write(pipe_fd[1], write_buf, sizeof(write_buf)) == -1)
        {
            perror("Write to pipe failed");
        }
        close(pipe_fd[1]);
    }
}
