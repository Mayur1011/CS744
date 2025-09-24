#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

void sigtstp_handler(int sig)
{
    printf("\nProcess stopping... (PID: %d)\n", getpid());
    fflush(stdout);

    // Reset to default handler and re-raise
    signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);
}

void sigcont_handler(int sig)
{
    printf("\nProcess resumed! (PID: %d)\n", getpid());
    fflush(stdout);

    // Reinstall SIGTSTP handler
    signal(SIGTSTP, sigtstp_handler);
}

int main()
{
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCONT, sigcont_handler);

    printf("Process started (PID: %d)\n", getpid());
    printf("Press Ctrl+Z to stop, then 'fg' or 'kill -CONT %d' to continue\n", getpid());

    int count = 0;
    while (1)
    {
        printf("Working... %d (time: %ld)\n", ++count, time(NULL));
        fflush(stdout);
        sleep(2);
    }

    return 0;
}