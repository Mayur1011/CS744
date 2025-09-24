#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

static void on_alarm(int signo)
{
    (void)signo;
    const char msg[] = "tick\n";
    write(STDOUT_FILENO, msg, sizeof msg - 1);
}

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    struct itimerval it = {0};
    it.it_value.tv_sec = 1;    // first expiry after 1s
    it.it_interval.tv_sec = 5; // then every 5s
    setitimer(ITIMER_REAL, &it, NULL);

    for (;;)
        pause(); // handler prints "tick" every second
}

/*
void alarm_handler(int sig) {
    static int count = 0;
    printf("Timer expired: %d\n", ++count);
}

int main() {
    signal(SIGALRM, alarm_handler);

    // alarm() limitations:
    // 1. Only seconds precision
    // 2. Only one-shot timer
    // 3. Only one timer per process

    printf("=== Using alarm() - one shot ===\n");
    alarm(2);
    sleep(5);

    printf("\n=== Using setitimer() - repeating ===\n");
    struct itimerval timer;

    // Set timer: 1.5 seconds initial, then every 1 second
    timer.it_value.tv_sec = 1;      // Initial delay
    timer.it_value.tv_usec = 500000; // 0.5 seconds
    timer.it_interval.tv_sec = 1;    // Repeat interval
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    sleep(6);

    // Stop timer
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

    return 0;
}

*/