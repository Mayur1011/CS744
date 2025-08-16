#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int k = 0;
    scanf("%d", &k);
    printf("%d\n", k);
    printf("Main process id is : %d\n", getpid());
    for (int i = 0; i < k; i++) {
        pid_t fork_result = fork();
        if (fork_result == -1) {
            perror("Fork Failed");
            exit(EXIT_FAILURE);
        }
        else if (fork_result == 0){
           printf("PID: %d PPID: %d created\n", getpid(), getppid()); 
        } else {
            waitpid(fork_result, NULL, 0);
            printf("PID: %d PPID: %d exited\n", fork_result, getpid());
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}
