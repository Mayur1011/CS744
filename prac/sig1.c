/*
Task: Write a program where the parent sends SIGUSR1 to the child, and the child, upon receiving it, prints a message and responds back with SIGUSR2 to the parent.
Use safe signal handling (sigaction() preferred).
Use raise(), kill(), getpid(), and the signal handler.
Make it repeat a few times (e.g., 3 ping-pongs), then both exit gracefully.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

void child_signal_handler(int signum)
{
    printf("Child process received signal %d\n", signum);
    kill(getppid(), SIGUSR1);
    exit(0);
}

void parent_signal_handler(int signum)
{
    printf("Parent process received signal %d\n", signum);
}

int main()
{
    int child = fork();
    if (child == 0)
    {
        signal(SIGUSR1, child_signal_handler);
        while (1)
            pause();
    }
    else
    {
        signal(SIGUSR1, parent_signal_handler);
        sleep(1);
        if (kill(child, SIGUSR1) == -1)
        {
            perror("kill failed");
            exit(EXIT_FAILURE);
        }
        int status;
        if (waitpid(child, &status, 0) == -1)
        {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status))
        {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}
