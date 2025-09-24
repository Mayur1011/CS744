#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void alarm_handler(int sig)
{
    printf("SIGALRM received during system call\n");
}

void test_system_call_interruption()
{
    printf("=== Testing system call interruption ===\n");

    // Method 1: Using signal() - system calls NOT restarted
    // signal(SIGALRM, alarm_handler);

    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    alarm(2);
    printf("Calling read() - will be interrupted by SIGALRM\n");

    char buffer[100];
    ssize_t result = read(STDIN_FILENO, buffer, sizeof(buffer));

    if (result == -1)
    {
        printf("read() interrupted: %s\n", strerror(errno));
    }
    else
    {
        printf("read() completed with %zd bytes\n", result);
        buffer[result] = '\0';
        printf("Data: %s", buffer);
    }
}

void test_system_call_restart()
{
    printf("\n=== Testing system call restart ===\n");

    // Method 2: Using sigaction() with SA_RESTART
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls
    sigemptyset(&sa.sa_mask);

    sigaction(SIGALRM, &sa, NULL);

    alarm(2);
    printf("Calling read() - will be restarted after SIGALRM\n");

    char buffer[100];
    ssize_t result = read(STDIN_FILENO, buffer, sizeof(buffer));

    if (result == -1)
    {
        printf("read() failed: %s\n", strerror(errno));
    }
    else
    {
        printf("read() completed with %zd bytes\n", result);
        buffer[result] = '\0';
        printf("Data: %s", buffer);
    }
}

int main()
{
    // test_system_call_interruption();
    test_system_call_restart();

    return 0;
}