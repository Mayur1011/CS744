#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void worker_sigcont_handler(int sig)
{
    printf("Worker %d: Resumed work\n", getpid());
}

void worker_process()
{
    signal(SIGCONT, worker_sigcont_handler);

    int work_units = 0;
    while (work_units < 10)
    {
        printf("Worker %d: Processing unit %d\n", getpid(), ++work_units);
        sleep(1);
    }
    printf("Worker %d: Finished all work\n", getpid());
}

int main()
{
    pid_t workers[3];

    // Create 3 worker processes
    for (int i = 0; i < 3; i++)
    {
        if ((workers[i] = fork()) == 0)
        {
            worker_process();
            return 0;
        }
    }

    sleep(1);

    // Stop all workers
    printf("Manager: Stopping all workers\n");
    for (int i = 0; i < 3; i++)
    {
        kill(workers[i], SIGSTOP);
    }

    sleep(3);

    // Resume workers one by one
    for (int i = 0; i < 3; i++)
    {
        printf("Manager: Resuming worker %d\n", workers[i]);
        kill(workers[i], SIGCONT);
        sleep(2); // Stagger the resumption
    }

    // Wait for all workers to complete
    for (int i = 0; i < 3; i++)
    {
        waitpid(workers[i], NULL, 0);
    }

    printf("Manager: All workers completed\n");
    return 0;
}