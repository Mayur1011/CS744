#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

char *const PATH_ENV_VAR = "/usr/bin:/usr/local/bin";

char **splitCommandIntoTokens(const char *command, int *numTokens)
{
  if (command == NULL)
  {
    *numTokens = 0;
    return NULL;
  }

  char *commandCopy = strdup(command);
  int arrayCapacity = 10;
  char **tokens = (char **)malloc(arrayCapacity * sizeof(char *));

  char *currToken = strtok(commandCopy, " ");
  *numTokens = 0;
  while (currToken != NULL)
  {
    if (*numTokens >= arrayCapacity)
    {
      arrayCapacity <<= 1;
      tokens = (char **)realloc(tokens, arrayCapacity * (sizeof(char *)));
    }
    tokens[(*numTokens)++] = strdup(currToken);
    currToken = strtok(NULL, " ");
  }

  if (*numTokens >= arrayCapacity)
  {
    arrayCapacity += 1;
    tokens = (char **)realloc(tokens, arrayCapacity * (sizeof(char *)));
  }
  tokens[*numTokens] = NULL;

  free(commandCopy);

  return tokens;
}

void printError(const char *command)
{
  fprintf(stderr, "%s: command not found\n", command);
}

void executeExitCommand()
{
  exit(0);
}

void executeEchoCommand(const char *command)
{
  if (strlen(command) > 5)
    printf("%s\n", command + 5);
  else
    printf("\n");
}

int checkRedirectionMode(char **tokens)
{
  int redirectionMode = 0;
  int tokenNo = 0;
  while (tokens[tokenNo] != NULL)
  {
    if (strcmp(tokens[tokenNo], ">") == 0 || strcmp(tokens[tokenNo], "1>") == 0)
    {
      redirectionMode = 1;
      break;
    }
    tokenNo++;
  }
  return redirectionMode;
}

int checkAppendRedirectionMode(char **tokens)
{
  int redirectionMode = 0;
  int tokenNo = 0;
  while (tokens[tokenNo] != NULL)
  {
    if (strcmp(tokens[tokenNo], ">>") == 0 || strcmp(tokens[tokenNo], "1>>") == 0)
    {
      redirectionMode = 1;
      break;
    }
    tokenNo++;
  }
  return redirectionMode;
}

void executeStdoutAppend(char **tokens, const char *outputFile)
{
  pid_t childPid = fork();
  if (childPid == 0)
  {
    if (outputFile != NULL)
    {
      int file_fd = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (file_fd < 0)
      {
        perror("Error opening output file in function executeStdoutRedirectionCommand");
        exit(1);
      }
      // redirect stdout to the file
      dup2(file_fd, STDOUT_FILENO);
      close(file_fd);
    }

    if (execvp(tokens[0], tokens) == -1)
    {
      printError(tokens[0]);
      exit(1);
    }
  }
  else if (childPid > 0)
  {
    waitpid(childPid, NULL, 0);
  }
  return;
}
void executeStdoutRedirection(char **tokens, const char *outputFile)
{
  pid_t childPid = fork();
  if (childPid == 0)
  {
    if (outputFile != NULL)
    {
      int file_fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (file_fd < 0)
      {
        perror("Error opening output file in function executeStdoutRedirectionCommand");
        exit(1);
      }
      // redirect stdout to the file
      dup2(file_fd, STDOUT_FILENO);
      close(file_fd);
    }

    if (execvp(tokens[0], tokens) == -1)
    {
      printError(tokens[0]);
      exit(1);
    }
  }
  else if (childPid > 0)
  {
    waitpid(childPid, NULL, 0);
  }
  return;
}

void evalCommand(const char *command)
{

  if (strncmp(command, "exit", 4) == 0)
    executeExitCommand();
  else if (checkRedirectionMode(splitCommandIntoTokens(command, &(int){0})) == 1)
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    char *outputFile = NULL;
    int tokenNo = 0;
    while (tokens[tokenNo] != NULL)
    {
      if (strcmp(tokens[tokenNo], ">") == 0 || strcmp(tokens[tokenNo], "1>") == 0)
      {
        outputFile = tokens[tokenNo + 1];
        tokens[tokenNo] = NULL; // command before > or 1>
        break;
      }
      tokenNo++;
    }
    if (outputFile == NULL)
    {
      fprintf(stderr, "Error: output file not specified for output redirection\n");
      return;
    }

    executeStdoutRedirection(tokens, outputFile);
    free(tokens);
  }
  else if (checkAppendRedirectionMode(splitCommandIntoTokens(command, &(int){0})) == 1)
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    char *outputFile = NULL;
    int tokenNo = 0;
    while (tokens[tokenNo] != NULL)
    {
      if (strcmp(tokens[tokenNo], ">>") == 0 || strcmp(tokens[tokenNo], "1>>") == 0)
      {
        outputFile = tokens[tokenNo + 1];
        tokens[tokenNo] = NULL; // command before >> or 1>>
        break;
      }
      tokenNo++;
    }
    if (outputFile == NULL)
    {
      fprintf(stderr, "Error: output file not specified for output redirection\n");
      return;
    }
    executeStdoutAppend(tokens, outputFile);
    free(tokens);
  }
  else if (strncmp(command, "echo", 4) == 0)
  {
    // TODO: read about sscanf
    // sscanf(command, "echo %[^\n]", command);

    executeEchoCommand(command);
  }
  else
  {
    printError(command);
  }
}

// search for matching command executable in PATH and current directory
char *completeCommandExecutable(const char *text, int state)
{
  static int textLen = 0;
  static char *matches = NULL;
  static char *currentDir = NULL;
  static DIR *dir = NULL;
  if (state == 0)
  {
    // this is the first call to tab completion
    textLen = strlen(text);
    if (matches != NULL)
      free(matches);

    // Create a list of directories to search for executables
    char *envPath = PATH_ENV_VAR;
    if (envPath != NULL)
    {
      matches = malloc(strlen(envPath) + 3);
      strcpy(matches, envPath);
      strcat(matches, ":.");
    }
    else
    {
      matches = strdup(".");
    }
    currentDir = strtok(matches, ":");
    if (dir)
      closedir(dir);
    dir = currentDir ? opendir(currentDir) : NULL;
  }

  while (currentDir != NULL)
  {
    if (dir == NULL)
    {
      currentDir = strtok(NULL, ":");
      if (dir)
        closedir(dir);
      continue;
    }
    // Read each files in the current directory and match the prefix
    struct dirent *currentFile;
    while ((currentFile = readdir(dir)) != NULL)
    {
      if (strncmp(currentFile->d_name, text, textLen) == 0)
      {
        if (strcmp(currentFile->d_name, ".") == 0 || strcmp(currentFile->d_name, "..") == 0)
          continue;

        // check if the current file is executable
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", currentDir, currentFile->d_name);
        if (access(filePath, X_OK) == 0)
          return strdup(currentFile->d_name);
      }
    }
    closedir(dir);
    currentDir = strtok(NULL, ":");
    dir = currentDir ? opendir(currentDir) : NULL;
  }

  return NULL;
}

char **tabCompletion(const char *text, int start, int end)
{
  rl_attempted_completion_over = 1; // don't use default filename completion

  return rl_completion_matches(text, completeCommandExecutable);

  // If you want tab completion for first word
  // if (start == 0)
  //   return rl_completion_matches(text, completeCommandExecutable);
  // else
  //   return NULL;
}

int main(int argc, char *argv[])
{
  setbuf(stdout, NULL); // Flush after every printf

  // tab completion callback function
  rl_attempted_completion_function = tabCompletion;

  char *command = NULL;

  while (1)
  {
    command = readline("$ ");

    if (command == NULL)
      break;

    if (command[0] == '\0')
    {
      free(command);
      continue;
    }

    add_history(command);
    evalCommand(command);
    free(command);
  }
  return 0;
}
