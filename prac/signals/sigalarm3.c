#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void alarm_handler(int sig)
{
    printf("PID %d: SIGALRM received\n", getpid());
}

int main()
{
    signal(SIGALRM, alarm_handler);

    // Edge Case 1: alarm(0) cancels timer
    // printf("=== Testing alarm(0) ===\n");
    // alarm(5);
    // sleep(1);
    // unsigned int remaining = alarm(0);
    // printf("Cancelled timer with %u seconds remaining\n", remaining);

    // Edge Case 2: Fork and alarm inheritance
    printf("\n=== Testing fork behavior ===\n");
    // alarm(3);

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        printf("Child: alarm inherited? Let's see...\n");
        sleep(5); // Will child get SIGALRM?
        printf("Child: Still alive after 5 seconds\n");
        return 0;
    }
    else
    {
        // Parent process
        alarm(3);
        printf("Parent %d: waiting for child and alarm...\n", getpid());
        wait(NULL);
        sleep(2); // Parent should get SIGALRM
    }

    // Edge Case 3: Exec and alarms
    // printf("\n=== Testing exec behavior ===\n");
    // alarm(2);

    // pid = fork();
    // if (pid == 0)
    // {
    //     printf("Child: About to exec, alarm should be cancelled\n");
    //     execl("/bin/echo", "echo", "After exec - no alarm here", NULL);
    //     return 1;
    // }
    // else
    // {
    //     wait(NULL);
    //     sleep(3); // Parent still has alarm
    // }

    // Edge Case 4: Signal mask and SIGALRM
    // printf("\n=== Testing signal masking ===\n");
    // sigset_t mask;
    // sigemptyset(&mask);
    // sigaddset(&mask, SIGALRM);

    // printf("Blocking SIGALRM...\n");
    // sigprocmask(SIG_BLOCK, &mask, NULL);

    // alarm(2);
    // sleep(3); // SIGALRM is blocked

    // printf("Unblocking SIGALRM...\n");
    // sigprocmask(SIG_UNBLOCK, &mask, NULL);
    // // Pending SIGALRM should be delivered now

    // sleep(1);

    return 0;
}