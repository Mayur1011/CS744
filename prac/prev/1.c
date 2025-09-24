/*
Write a program that accepts an integer k as a command-line argument and spawns k
child processes. The parent process should wait for each child to finish, reaping the child
processes in the order they were created. After reaping each child, the parent should
print the PID of the reaped child and then exit. The child should print its PID, sleep for 2
seconds, and exit.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int k;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <number_of_children>\n", argv[0]);
        return EXIT_FAILURE;
    }
    k = atoi(argv[1]);

    if (k <= 0)
    {
        fprintf(stderr, "Invalid number of children: %d\n", k);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < k; i++)
    {
        pid_t child_pid = fork();

        if (child_pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (child_pid == 0)
        {
            printf("Child PID: %d\n", getpid());
            sleep(2);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Dont wait if you want parallelism
            waitpid(child_pid, NULL, 0);
            printf("Reaped child PID: %d\n", child_pid);
        }
    }
}