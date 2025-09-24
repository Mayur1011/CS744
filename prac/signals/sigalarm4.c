/*

Problem Statement:
Create a "watchdog" program that monitors a single child process. The child process's behavior is determined by a command-line argument.
The watchdog program takes two arguments: a timeout in seconds and a mode (normal, infinite, or segfault).

The watchdog forks a child process.
The watchdog sets an alarm for the specified timeout duration. If the alarm goes off before the child has terminated, the watchdog's alarm handler must kill the child process using SIGKILL.

The child process's behavior is as follows:
- If the mode is normal, it prints its PID, sleeps for 1 second, and exits normally with status 0.
- If the mode is infinite, it prints its PID and enters an infinite loop.
- If the mode is segfault, it prints its PID and then deliberately causes a segmentation fault (e.g., by writing to a NULL pointer).

The watchdog parent process must wait() for the child. After the wait() call returns, it must determine how the child terminated and print one of the following messages:
- "SUCCESS: Child [PID] exited normally with status [status]."
- "FAILURE: Child [PID] was terminated by signal [signal_number]."
- "TIMEOUT: Child [PID] was killed by the watchdog." (This message should be printed if the child was killed by the watchdog's alarm handler).

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

pid_t child_pid = -1;
volatile int timeout_expired = 0;

void alarm_handler(int signum)
{
    // TODO:
    // 1. Set the global 'timeout_expired' flag to 1.
    timeout_expired = 1;
    // 2. If the child process exists (child_pid > 0), kill it with SIGKILL.
    if (child_pid > 0)
    {
        kill(child_pid, SIGKILL);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <timeout_sec> <normal|infinite|segfault>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int timeout = atoi(argv[1]);
    char *mode = argv[2];

    signal(SIGALRM, alarm_handler);

    child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    { // Child process
        printf("Child process started with PID: %d\n", getpid());
        if (strcmp(mode, "normal") == 0)
        {
            sleep(1);
            exit(0);
        }
        else if (strcmp(mode, "infinite") == 0)
        {
            while (1)
                ;
        }
        else if (strcmp(mode, "segfault") == 0)
        {
            char *ptr = NULL;
            *ptr = 'A';
        }
        else
        {
            fprintf(stderr, "Invalid mode for child.\n");
            exit(1);
        }
    }
    else
    { // Parent process (Watchdog)
        alarm(timeout);
        int status;

        // TODO:
        // 1. Wait for the child process to terminate.
        waitpid(child_pid, &status, 0);
        // 2. After wait returns, cancel any pending alarm (using alarm(0)).
        alarm(0);
        // 3. Check the 'timeout_expired' flag. If it's set, print the TIMEOUT message.
        if (timeout_expired)
        {
            printf("TIMEOUT: Child %d was killed by the watchdog.\n", child_pid);
        }
        // 4. If not a timeout, use the status macros (WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG)
        //    to determine how the child died and print the appropriate SUCCESS or FAILURE message.
        else
        {
            if (WIFEXITED(status))
            {
                printf("SUCCESS: Child %d exited normally with status %d.\n", child_pid, WEXITSTATUS(status));
            }
            else
            {
                printf("FAILURE: Child %d was terminated by signal %d.\n", child_pid, WTERMSIG(status));
            }
        }
    }

    return 0;
}