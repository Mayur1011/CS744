#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile int signal_count = 0;

void handler(int sig)
{
    signal_count++;
    // sleep(1); // Simulate slow handler
    printf("Handled signal with signal_count : %d\n", signal_count);
}

int main()
{
    signal(SIGUSR1, handler);

    // Send multiple signals rapidly
    for (int i = 0; i < 10; i++)
    {
        kill(getpid(), SIGUSR1);
    }

    // sleep(15);
    printf("Signals handled: %d\n", signal_count);

    return 0;
}