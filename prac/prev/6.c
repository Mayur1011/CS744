/*
Write a program that creates three child processes; child1 will take integer as an input from stdin and write it to the pipe connected with child2, child2 takes another integer as input from stdin, and then transfers both integers to child3 using another pipe. child3 performs addition on the two integer values and prints result to stdout.

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    int pipe_fd1[2];
    int pipe_fd2[2];

    pipe(pipe_fd1);
    pipe(pipe_fd2);

    if (fork() == 0)
    {
        close(pipe_fd1[0]);
        int num1;
        printf("Enter first integer: ");
        scanf("%d", &num1);
        write(pipe_fd1[1], &num1, sizeof(num1));
        close(pipe_fd1[1]);
        exit(0);
    }

    if (fork() == 0)
    {
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);

        int num1, num2;
        read(pipe_fd1[0], &num1, sizeof(num1));
        printf("Enter second integer: ");
        scanf("%d", &num2);
        write(pipe_fd2[1], &num1, sizeof(num1));
        write(pipe_fd2[1], &num2, sizeof(num2));
        close(pipe_fd2[1]);
        exit(0);
    }

    if (fork() == 0)
    {
        close(pipe_fd1[1]);
        close(pipe_fd1[0]);
        close(pipe_fd2[1]);
        int num1, num2;
        read(pipe_fd2[0], &num1, sizeof(num1));
        read(pipe_fd2[0], &num2, sizeof(num2));
        printf("Sum: %d\n", num1 + num2);
        close(pipe_fd2[0]);
        exit(0);
    }

    close(pipe_fd1[0]);
    close(pipe_fd1[1]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);

    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}