#include <signal.h>
#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int child_process[64];
volatile int child_process_idx = 0; 
int foreground_pid = -1;

/* 
 * Splits the string by space and returns the array of tokens
*/
char **tokenize(char *line) {
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for(i =0; i < strlen(line); i++){

        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
            token[tokenIndex] = '\0';
            // printf("%s\n", token);
            if (tokenIndex != 0){
                tokens[tokenNo] = (char*) malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0; 
            }
        } else {
            token[tokenIndex++] = readChar;
        }
    }

    free(token);
    tokens[tokenNo] = NULL ;
    return tokens;
}

void update_child_process_array(int process_id) {
    for (int i = 0; i < child_process_idx; i++) {
        if(child_process[i] == process_id) {
            for (int j = i; j < child_process_idx - 1; j++) child_process[j] = child_process[j + 1];
            break;
        }
    }
    child_process_idx -= 1;
}

void sigint_handler(int signum) {
    printf("sighandler called");
    kill(-foreground_pid, SIGINT); // send this signal to all the proess in the process group
}


void execute_command(char** tokens, int no_of_tokens) {
    // for (int i = 0; tokens[i] != NULL; i++) printf("%s\n", tokens[i]);
    if (tokens[0] == NULL) {
        // fprintf(stderr, "no such command\n");
    }
    else if (strcmp(tokens[0], "cd") == 0) {
        if (tokens[1] == NULL) {
            const char* home_dir = getenv("HOME");
            if(chdir(home_dir) != 0) perror("error while changing directory to HOME");
        }
        else if (chdir(tokens[1]) != 0) {
            perror("");
        }
    } 
    else if(strcmp(tokens[0], "exit") == 0) {
        // send kill signal to all process
        for (int i = 0; i < child_process_idx; i++) kill(child_process[i], SIGKILL);
        kill(-getpid(), SIGKILL);
    }
    else if(no_of_tokens >= 3 && strcmp(tokens[no_of_tokens - 2], "&") == 0) {
        // process to run in background
        tokens[no_of_tokens - 2] = NULL;
        pid_t fork_result = fork();
        if (child_process_idx < 64) { child_process[child_process_idx++] = fork_result; }
        if(fork_result == -1) fprintf(stderr, "Fork failed\n"); 
        if(fork_result == 0) {
            if(execvp(tokens[0], tokens) == -1){
                perror("Command not found");
                exit(EXIT_SUCCESS);
            }
        } 
    } 
    else {
        pid_t fork_result = fork();
        signal(SIGINT, sigint_handler);
        if (fork_result == -1) fprintf(stderr, "Fork failed\n");
        if (fork_result == 0) {
            setpgid(0, 0); // new process group for child
            if (execvp(tokens[0], tokens) == -1) {
                fprintf(stderr, "Command not found\n");
                exit(EXIT_SUCCESS);
            }
        } else if (fork_result > 0) {
            foreground_pid = fork_result;
            setpgid(fork_result, fork_result);
            waitpid(fork_result, NULL, 0);
            // foreground_pid = -1;
            signal(SIGINT, SIG_IGN);
        }
    }
}

int main(int argc, char* argv[]) {
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;
    signal(SIGINT, SIG_IGN);
    // signal(SIGINT, sigint_handler);

    while(1) {
        // Delete all the zombie process
        while(1) {
            int killed_process_id = waitpid(-1, NULL, WNOHANG);
            if(killed_process_id <= 0) break;
            printf("Shell : background process with %d finished\n", killed_process_id);
            update_child_process_array(killed_process_id);
        }

        /* BEGIN: TAKING INPUT */
        bzero(line, sizeof(line));
        char getcwd_buf[100000];
        getcwd(getcwd_buf, 100000);
        printf("%s ", getcwd_buf);
        printf("$ ");
        scanf("%[^\n]", line);
        getchar();

        // printf("Command entered: %s (remove this debug output later)\n", line);
        /* END: TAKING INPUT */

        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);

        // for(i=0;tokens[i]!=NULL;i++){
        //     printf("found token %s (remove this debug output later)\n", tokens[i]);
        // }

        char** command = (char**) malloc(sizeof(char*) * MAX_NUM_TOKENS);
        int command_index = 0;
        for  (int i = 0; tokens[i] != NULL; i++) {
            command[command_index] = (char*) malloc(sizeof(char) * MAX_TOKEN_SIZE);
            if (strcmp(tokens[i], "&&") == 0 || strcmp(tokens[i], "|") == 0) {
                command[command_index] = NULL;
                execute_command(command, command_index + 1);
                command_index = 0;
            }
            else command[command_index++] = tokens[i];
        }

        // last command
        command[command_index] = NULL;
        execute_command(command, command_index + 1);


        // printf("No of child process running : %d \n", child_process_idx);
        // for (int i = 0; i < child_process_idx; i++) printf("%d, ", child_process[i]);

        // Freeing the allocated memory	
        for(i = 0; tokens[i] != NULL; i++) free(tokens[i]);
        free(command);
        free(tokens);

    }
    return 0;
}
