#include <signal.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

volatile sig_atomic_t leaf_wait = 1;
pid_t child1_pid = 0;
pid_t child2_pid = 0;
pid_t curr_pid = 0;

void sig_handler(int signum)
{
    if (child1_pid > 0 && child2_pid > 0)
    {
        kill(child1_pid, SIGTERM);
        kill(child2_pid, SIGTERM);
        waitpid(child1_pid, NULL, 0);
        printf("PID : %d reaped\n", child1_pid);
        waitpid(child2_pid, NULL, 0);
        printf("PID : %d reaped\n", child2_pid);
    }
    leaf_wait = 0;
}

int main()
{
    int n = 3;
    pid_t parent_pid = getpid();
    n--;
    signal(SIGTERM, sig_handler);
    while (n--)
    {
        pid_t child1 = fork();
        if (child1 == 0)
        {
            continue;
        }
        else
        {
            pid_t child2 = fork();
            if (child2 > 0)
            {
                child1_pid = child1;
                child2_pid = child2;
                curr_pid = getpid();
                printf("PID : %d , child PID : %d\n", getpid(), child1);
                printf("PID : %d , child PID : %d\n", getpid(), child2);
                if (getpid() == parent_pid)
                {
                    sleep(2);
                    printf("Waiting for input : \n");
                    getchar();
                    kill(curr_pid, SIGTERM);
                }
                break;
            }
        }
    }
    while (leaf_wait)
    {
    }
}
