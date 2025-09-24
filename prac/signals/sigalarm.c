#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/*
Only ONE alarm can be active per process
Default action: terminate the process
Timer continues even if process is sleeping/blocked
*/

static void alarm_handler(int sig)
{
    // char buffer[50];
    // int len = snprintf(buffer, sizeof(buffer), "Alarm received: %d\n", sig);
    // write(STDOUT_FILENO, buffer, len);

    printf("Alarm received\n");
}

int main()
{
    // signal(SIGALRM, alarm_handler);
    struct sigaction sa = {0};
    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;

    // MASKING is all about DO NOT DISTURB (like block the following signals when alarm_handler is running)'
    sigemptyset(&sa.sa_mask);
    // sigaddset(&sa.sa_mask, SIGINT);

    sigaction(SIGALRM, &sa, NULL);

    int threshold_curr = 0;
    int threshold_max = 5;

    while (threshold_curr < threshold_max)
    {
        alarm(3); // Schedule an alarm in 3 seconds
        pause();
        printf("Current threshold: %d\n", threshold_curr);
        threshold_curr++;
    }

    printf("Process exits here\n");
    return 0;
}