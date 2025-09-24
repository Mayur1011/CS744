#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Global variable to hold the child's PID
pid_t child_pid = 0;
pid_t first_process = 0;

void signal_handler(int signum)
{
    // TODO:
    // 1. Print a message indicating the signal was received.
    // 2. If this process is not the first process in the chain (i.e., its parent is not the shell),
    //    it should send the same signal (SIGUSR1) to its parent.
    // 3. The process should then wait for its child to terminate.
    // 4. After the child terminates, print a reaping message.
    //    Example: "PID [parent_pid] reaped child [child_pid]"

    printf("Signal received to process: %d\n", getpid());
    if (getppid() != first_process)
    {
        kill(getppid(), SIGUSR1);
    }
    if (child_pid > 0)
    {
        waitpid(child_pid, NULL, 0);
        printf("PID %d reaped child %d\n", getpid(), child_pid);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <n_processes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    if (n <= 1)
    {
        fprintf(stderr, "Number of processes must be greater than 1.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    first_process = getppid();
    printf("Gen 1 (PID %d), Parent: %d\n", getpid(), getppid());

    // TODO:
    // 1. Set up the signal handler for SIGUSR1.
    signal(SIGUSR1, signal_handler);
    // 2. Create the process chain of length 'n' using a loop and fork().
    //    - In each iteration, the child process should break from the loop and continue to the next iteration
    //      to create its own child.
    //    - The parent process should store the child's PID and then break the loop.
    //    - Each new process should print its generation number, PID, and its parent's PID.
    for (int i = 1; i < n; i++)
    {
        child_pid = fork();
        if (child_pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (child_pid == 0)
        {
            printf("Gen %d (PID %d), Parent: %d\n", i + 1, getpid(), getppid());
        }
        else if (child_pid > 0)
        {
            break;
        }
    }

    // After the chain is created:
    if (child_pid == 0)
    {
        // This is the last process in the chain.
        // TODO:
        // 1. Sleep for a moment to ensure the entire chain is set up.
        sleep(1);
        // 2. Send SIGUSR1 to its parent.
        kill(getppid(), SIGUSR1);
        printf("Last process (%d) sending signal to parent %d\n", getpid(), getppid());
        exit(0);
    }
    else
    {
        // All other processes in the chain simply wait for the signal.
        // The signal handler will do the work.
        pause(); // Wait for a signal
    }

    return 0;
}