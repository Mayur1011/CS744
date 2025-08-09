#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t fork_result = fork();
    if (fork_result == 0) {
        printf("%s, : %d\n", "I am child", (int)getpid());
        exit(EXIT_SUCCESS);
    } else {
        printf("%s : %d\n", "I am parent", (int)getpid());
        waitpid(fork_result, NULL, 0);
    }
    return 0;
}
