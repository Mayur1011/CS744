#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_HISTORY_SIZE 20
#define HISTORY_FILE ".my_shell_history"


// --- Global Variables ---
int child_process[64] = {0};
volatile int child_process_idx = 0; 
int foreground_pid = -1;
int pipe_fd[2] = {-1};

// --- History Feature Global Variables ---
char *history[MAX_HISTORY_SIZE];
int history_count = 0; // Number of items currently in history (caps at MAX_HISTORY_SIZE)
int history_idx = 0;   // Next index to write to in the circular buffer

// --- Forward Declarations for History Functions ---
void add_to_history(const char *line);
void print_history();
void load_history();
void save_history();
void free_history();


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
    if (foreground_pid > 0) {
        kill(-foreground_pid, SIGINT); // send this signal to all the proess in the process group
    }
}

void execute_pipe_command(char ***commands, int no_of_commands) {
    int prev_read_fd = -1; 
    int c_process[no_of_commands];

    for (int i = 0; i < no_of_commands; i++) {
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
            if (i > 0) {
                if (dup2(prev_read_fd, STDIN_FILENO) == -1) { 
                    perror("dup error"); 
                    exit(EXIT_FAILURE);
                }
            }
            if (i < no_of_commands - 1) {
                if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) { 
                    perror("dup error"); 
                    exit(EXIT_FAILURE);
                }
            }

            if(prev_read_fd != -1) close(prev_read_fd);
            if(pipe_fd[0] != -1) close(pipe_fd[0]);
            if(pipe_fd[1] != -1) close(pipe_fd[1]);

            execvp(commands[i][0], commands[i]);
            perror("Exec failed");
            exit(EXIT_FAILURE);
        }

        if (pipe_fd[1] != -1) close(pipe_fd[1]);
        if (prev_read_fd != -1) close(prev_read_fd);
        prev_read_fd = pipe_fd[0];
        c_process[i] = fork_result;
    }

    for(int i = 0; i < no_of_commands; i++) waitpid(c_process[i], NULL, 0);
}

void execute_command(char** tokens, int no_of_tokens) {
    if (tokens[0] == NULL) {
        // No command entered
    }
    else if (strcmp(tokens[0], "cd") == 0) {
        if (tokens[1] == NULL) {
            const char* home_dir = getenv("HOME");
            if(home_dir && chdir(home_dir) != 0) perror("error while changing directory to HOME");
        }
        else if (chdir(tokens[1]) != 0) {
            perror("cd failed");
        }
    } 
    else if (strcmp(tokens[0], "history") == 0) {
        print_history();
    }
    else if(strcmp(tokens[0], "exit") == 0) {
        save_history();
        free_history();
        kill(-getpid(), SIGKILL);
    }
    else if(no_of_tokens >= 2 && strcmp(tokens[no_of_tokens - 2], "&") == 0) {
        if (child_process_idx >= 64) {
            perror("Background process limit reached");
            return;
        }
        tokens[no_of_tokens - 2] = NULL;
        pid_t fork_result = fork();
        if (child_process_idx < 64) { child_process[child_process_idx++] = fork_result; }
        if(fork_result == -1) fprintf(stderr, "Fork failed\n"); 
        if(fork_result == 0) {
            if(execvp(tokens[0], tokens) == -1){
                perror("Command not found");
                _exit(EXIT_FAILURE);
            }
        } 
    } 
    else {
        pid_t fork_result = fork();
        signal(SIGINT, sigint_handler);
        if (fork_result == -1) fprintf(stderr, "Fork failed\n");
        if (fork_result == 0) {
            setpgid(0, 0); 
            if (execvp(tokens[0], tokens) == -1) {
                fprintf(stderr, "Command not found: %s\n", tokens[0]);
                _exit(EXIT_FAILURE);
            }
        } else if (fork_result > 0) {
            foreground_pid = fork_result;
            setpgid(fork_result, fork_result); 
            waitpid(fork_result, NULL, 0);
            foreground_pid = -1;
            signal(SIGINT, SIG_IGN);
        }
    }
}


// --- HISTORY FEATURE FUNCTIONS ---

/**
 * @brief Adds a command to the history circular buffer.
 * @param line The command line to add.
 */
void add_to_history(const char *line) {
    // Don't add empty commands or the history command itself
    char temp_line[MAX_INPUT_SIZE];
    strcpy(temp_line, line);
    temp_line[strcspn(temp_line, "\n")] = 0; // Remove newline for comparison
    if (temp_line[0] == '\0' || strcmp(temp_line, "history") == 0) {
        return;
    }

    // Free the oldest command if the buffer is full and we are about to overwrite it
    if (history[history_idx] != NULL) {
        free(history[history_idx]);
    }

    // Make a copy of the command and store it
    history[history_idx] = strdup(temp_line);

    // Move to the next index in the circular buffer
    history_idx = (history_idx + 1) % MAX_HISTORY_SIZE;

    // Increment the count, but cap it at the max size
    if (history_count < MAX_HISTORY_SIZE) {
        history_count++;
    }
}

/**
 * @brief Prints all commands currently stored in the history buffer.
 */
void print_history() {
    int start_point = 0;
    // If the buffer is full, the starting point is the next index to be written to.
    if (history_count == MAX_HISTORY_SIZE) {
        start_point = history_idx;
    }

    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        int current_idx = (start_point + i) % MAX_HISTORY_SIZE;
        printf("  %5d: %s\n", i + 1, history[current_idx]);
    }
}

/**
 * @brief Helper function to construct the full path to the history file.
 * @return A static character buffer containing the path.
 */
char* get_history_filepath() {
    static char path[MAX_INPUT_SIZE];
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        snprintf(path, sizeof(path), "%s/%s", home_dir, HISTORY_FILE);
        return path;
    }
    return NULL;
}

/**
 * @brief Loads command history from the history file into memory on shell startup.
 */
void load_history() {
    char* filepath = get_history_filepath();
    if (!filepath) return;

    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        return; // No history file yet, which is fine
    }

    char line[MAX_INPUT_SIZE];
    while (fgets(line, sizeof(line), file) != NULL) {
        add_to_history(line);
    }
    fclose(file);
}

/**
 * @brief Saves the current session's command history to the history file upon exit.
 */
void save_history() {
    char* filepath = get_history_filepath();
    if (!filepath) return;

    FILE* file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Failed to save history");
        return;
    }

    int start_point = 0;
    if (history_count == MAX_HISTORY_SIZE) {
        start_point = history_idx;
    }

    for (int i = 0; i < history_count; i++) {
        int current_idx = (start_point + i) % MAX_HISTORY_SIZE;
        fprintf(file, "%s\n", history[current_idx]);
    }

    fclose(file);
}

/**
 * @brief Frees all memory allocated for storing history commands.
 */
void free_history() {
    for (int i = 0; i < MAX_HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
            history[i] = NULL;
        }
    }
}


// --- MAIN FUNCTION ---
int main(int argc, char* argv[]) {
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i = 0;

    signal(SIGINT, SIG_IGN);
    load_history(); // Load previous session's history

    while(1) {
        while(1) {
            int killed_process_id = waitpid(-1, NULL, WNOHANG);
            if(killed_process_id <= 0) break;
            printf("Shell : background process with %d finished\n", killed_process_id);
            update_child_process_array(killed_process_id);
        }

        bzero(line, sizeof(line));
        char getcwd_buf[1024];
        if (getcwd(getcwd_buf, sizeof(getcwd_buf)) != NULL) {
            printf("%s", getcwd_buf);
        }
        printf("$ ");
        
        if (fgets(line, sizeof(line), stdin) == NULL) {
            // Handle Ctrl+D (EOF)
            printf("\nExiting shell.\n");
            break;
        }

        // Add command to history if it's not just whitespace
        if (strspn(line, " \t\n") != strlen(line)) {
            add_to_history(line);
        }
        
        tokens = tokenize(line);

        char** command = (char**) malloc(sizeof(char*) * MAX_NUM_TOKENS);
        char*** pipe_commands = (char***) malloc(sizeof(char*) * MAX_NUM_TOKENS);
        int command_index = 0;
        int pipe_command_number = 0;
        bool pipe_command = false;
        
        for  (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "&&") == 0) {
                command[command_index] = NULL;
                execute_command(command, command_index + 1);
                // Reset for the next command
                for(int j=0; j<command_index; j++) free(command[j]);
                command_index = 0;
            } else if (strcmp(tokens[i], "|") == 0) {
                pipe_command = true;
                command[command_index] = NULL;
                pipe_commands[pipe_command_number++] = command;
                command = (char**) malloc(sizeof(char*) * MAX_NUM_TOKENS);
                command_index = 0;
            } else {
                command[command_index++] = strdup(tokens[i]);
            }
        }

        command[command_index] = NULL;

        if (pipe_command == true) {
            pipe_commands[pipe_command_number++] = command;
            pipe_commands[pipe_command_number] = NULL;
            execute_pipe_command(pipe_commands, pipe_command_number);
            
            // Free memory for pipe commands
            for(int k=0; k<pipe_command_number; k++){
                for(int l=0; pipe_commands[k][l] != NULL; l++){
                    free(pipe_commands[k][l]);
                }
                free(pipe_commands[k]);
            }

        } else {
            execute_command(command, command_index + 1);
            // Free memory for single command
            for(int j=0; j<command_index; j++) free(command[j]);
        }

        free(pipe_commands);
        free(command);
        for(i = 0; tokens[i] != NULL; i++) free(tokens[i]);
        free(tokens);
    }

    // Cleanup before exiting
    save_history();
    free_history();
    return 0;
}
