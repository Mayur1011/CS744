#include <signal.h>
#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int child_process[64] = {0};
volatile int child_process_idx = 0; 
int foreground_pid = -1;
int pipe_fd[2] = {-1};

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
    kill(-foreground_pid, SIGINT); // send this signal to all the proess in the process group
}

// void execute_pipe_command(char** command, int command_number) {
//     pid_t fork_result = fork();
//     if (fork_result == 0) {
//         if (command_number == 1) {
//             // first command connect stdout to write fd
//             dup2(pipe_fd[1], STDOUT_FILENO);
//             close(pipe_fd[0]);
//             close(pipe_fd[1]); // Close this as dup2 have made a new fd
//         } else if (command_number == -1) {
//             // last command connect stdin to read fd
//             dup2(pipe_fd[0], STDIN_FILENO);
//             close(pipe_fd[1]);
//             close(pipe_fd[0]);
//         } else {
//             dup2(pipe_fd[0], STDIN_FILENO);
//             dup2(pipe_fd[1], STDOUT_FILENO);
//             close(pipe_fd[0]);
//             close(pipe_fd[1]);
//         }
//         execvp(command[0], command);
//         perror("Exec failed");
//         exit(EXIT_FAILURE);
//     } else {
//         close(pipe_fd[1]); // The reason I am closing the write end because I dont want the other process think that some process still wants to write something in the pipe and hence will be waiting for EOF.
//         // close(pipe_fd[0]);
//         waitpid(fork_result, NULL, 0);
//     }
// }

void execute_pipe_command(char ***commands, int no_of_commands) {

    int prev_read_fd = -1; // This is needed as we are reading the output of prev command(stored in previous pipe) as input for current command. 

    int c_process[no_of_commands];
    // printf("%d\n", no_of_commands);
    for (int i = 0; i < no_of_commands; i++) {

        // for (int j = 0; commands[i][j] != NULL; j++) printf("%s ", commands[i][j]);
        // printf("%d\n", no_of_commands);

        int pipe_fd[2] = {-1, -1};
        if (i < no_of_commands - 1) {
            if(pipe(pipe_fd) == -1) {
                perror("Pipe failed");
                exit(EXIT_FAILURE);
            };
        }
        
        pid_t fork_result = fork();

        if (fork_result < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (fork_result == 0) {
            // connect previous read fd to STDIN
            if (i > 0) {
                if (dup2(prev_read_fd, STDIN_FILENO) == -1) { 
                    perror("dup error"); 
                    exit(EXIT_FAILURE);
                }
            }

            // connect current write fd to STDOUT. As the current command will put its output in stdout but we want it to be stored in pipe.
            if (i < no_of_commands - 1) {if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) { 
                perror("dup error"); 
                exit(EXIT_FAILURE);
            }}

            // Now as we have made duplicated fds, we can close all other fds
            if(prev_read_fd != -1) close(prev_read_fd);
            if(pipe_fd[0] != -1) close(pipe_fd[0]);
            if(pipe_fd[1] != -1) close(pipe_fd[1]);

            execvp(commands[i][0], commands[i]);
            perror("Exec failed");
            exit(EXIT_FAILURE);
        }

        // waitpid(fork_result, NULL, 0);

        // Close the write end from parent otherwise the child might think that parent has to write something
        if (pipe_fd[1] != -1) close(pipe_fd[1]);

        // Close the prev_read_fd and also update it
        if (prev_read_fd != -1) close(prev_read_fd);
        prev_read_fd = pipe_fd[0];

        c_process[i] = fork_result;
    }

    for(int i = 0; i < no_of_commands; i++) waitpid(c_process[i], NULL, 0);
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
        kill(-getpid(), SIGKILL);
    }
    else if(no_of_tokens >= 3 && strcmp(tokens[no_of_tokens - 2], "&") == 0) {
        if (child_process_idx >= 64) {
            perror("Background process limit reached");
            return;
        }
        // process to run in background
        tokens[no_of_tokens - 2] = NULL;
        pid_t fork_result = fork();
        if(fork_result == -1) fprintf(stderr, "Fork failed\n"); 
        if (child_process_idx < 64) { child_process[child_process_idx++] = fork_result; }
        if(fork_result == 0) {
            if(execvp(tokens[0], tokens) == -1){
                perror("Command not found");
                _exit(EXIT_SUCCESS);
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
                _exit(EXIT_SUCCESS);
            }
        } else if (fork_result > 0) {
            foreground_pid = fork_result;
            setpgid(fork_result, fork_result); // not needed, just for safety
            waitpid(fork_result, NULL, 0);
            // foreground_pid = -1;
            signal(SIGINT, SIG_IGN);
        }
    }
}

int main(int argc, char* argv[]) {
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i = 0;
    bool pipe_command = false;
    signal(SIGINT, SIG_IGN);
    pipe(pipe_fd);
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

        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);

        // for(i=0;tokens[i]!=NULL;i++){
        //     printf("found token %s (remove this debug output later)\n", tokens[i]);
        // }

        char** command = (char**) malloc(sizeof(char*) * MAX_NUM_TOKENS);
        char*** pipe_commands = (char***) malloc(sizeof(char*) * MAX_NUM_TOKENS);
        int command_index = 0;
        int pipe_command_number = 0;
        pipe_command = false;
        for  (int i = 0; tokens[i] != NULL; i++) {
            command[command_index] = (char*) malloc(sizeof(char) * MAX_TOKEN_SIZE);
            if (strcmp(tokens[i], "&&") == 0) {
                command[command_index] = NULL;
                execute_command(command, command_index + 1);
                command_index = 0;
            } else if (strcmp(tokens[i], "|") == 0) {
                pipe_command = true;
                command[command_index] = NULL;
                pipe_commands[pipe_command_number++] = command;
                command = (char**) malloc(sizeof(char*) * MAX_NUM_TOKENS); // command pointing to new space
                command_index = 0;
            }
            else command[command_index++] = tokens[i];
        }

        if (pipe_command == true) {
            command[command_index] = NULL;
            pipe_commands[pipe_command_number++] = command;
            pipe_commands[pipe_command_number] = NULL;
            execute_pipe_command(pipe_commands, pipe_command_number);

        } else {
            command[command_index] = NULL;
            execute_command(command, command_index + 1);
        }

        // printf("No of child process running : %d \n", child_process_idx);
        // for (int i = 0; i < child_process_idx; i++) printf("%d, ", child_process[i]);

        // Freeing the allocated memory	
        for(i = 0; tokens[i] != NULL; i++) free(tokens[i]);
        free(command);
        free(tokens);

    }
    return 0;
}
