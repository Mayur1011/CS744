/*
Task: Fork a child that sleeps for a few seconds then exits. Use a SIGCHLD handler in the parent to print a message like "Child <pid> terminated" when the child finishes.
In the handler, reap the child using waitpid(..., WNOHANG) or a loop.
This demonstrates asynchronous notification of child termination.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

void sigchild_handler(int signum)
{
    pid_t child_pid;
    int status;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
        {
            printf("Child with PID : %d terminated with status %d\n", child_pid, WEXITSTATUS(status));
        }
    }
}

int main()
{
    pid_t child = fork();
    if (child == 0)
    {
        sleep(3);
        exit(100);
    }
    else
    {
        // Parent process
        signal(SIGCHLD, sigchild_handler);
        pause();
    }
    return 0;
}