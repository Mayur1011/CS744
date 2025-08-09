#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void) {
    pid_t fork_result = fork();
    if (fork_result == 0) {
        // execl("/usr/bin/ls", "ls", "-l", (char*)NULL);
        // execlp("ls", "ls", "-l", (char*)NULL);
        char* args[3];
        args[0] = "ls";
        args[1] = "-l";
        args[2] = NULL;
        
        // char* args[] = {"ls", "-l", NULL};

        execvp("ls", args);
    } else {
        waitpid(fork_result, NULL, 0);
    }
}
