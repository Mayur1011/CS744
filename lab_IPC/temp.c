#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sigusr1_handler(int sig)
{
    write(STDOUT_FILENO, "Caught SIGUSR1\n", 15);
}

void sigint_handler(int sig)
{
    printf("Caught SIGINT (%d)\n", sig);
    // raise(SIGUSR1);
    kill(getpid(), SIGUSR1);
    exit(0);
}

int main()
{
    signal(SIGINT, sigint_handler);
    signal(SIGUSR1, sigusr1_handler);
    while (1)
    {
        pause(); // Wait for signals
    }
    return 0;
}
