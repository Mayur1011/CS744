/*
Create a child process from a parent process and print both parent’s and child’s PIDs and PPIDs. The child’s PID should show the parent's PID correctly. Then, make the child process sleep for some time during which the parent process terminates and after waking up the child process prints the PID and PPID. The PPID of the child should have changed by now. Make observations on which process becomes the new parent of this child process (use ps to verify).
*/

#include <stdio.h>
#include <unistd.h>

int main()
{
    int pid = fork();
    if (pid == 0)
    {
        // Child process
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
        sleep(2);
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
    }
    else
    {
        // Parent process
        printf("Parent PID: %d, Child PID: %d\n", getpid(), pid);
        sleep(1);
    }
    return 0;
}