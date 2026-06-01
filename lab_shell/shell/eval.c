#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"

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

void executeCatCommand(const char *command)
{
  char **tokens = splitCommandIntoTokens(command, &(int){0});

  for (int i = 1; tokens[i] != NULL; i++)
  {
    FILE *file = fopen(tokens[i], "r");
    if (file == NULL)
    {
      fprintf(stderr, "cat: %s: No such file or directory\n", tokens[i]);
      continue;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL)
      printf("%s", buffer);
    fclose(file);
  }

  free(tokens);
}

void executeGeneralCommand(char **tokens)
{
  pid_t childPid = fork();
  if (childPid == 0)
  {
    if (execvp(tokens[0], tokens) == -1)
    {
      printError(tokens[0]);
      exit(1);
    }
  }
  else
  {
    waitpid(childPid, NULL, 0);
  }
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
  else if (strncmp(command, "cat", 3) == 0)
    executeCatCommand(command);
  else if (strncmp(command, "echo", 4) == 0)
  {
    // TODO: read about sscanf
    // sscanf(command, "echo %[^\n]", command);

    executeEchoCommand(command);
  }
  else
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    if (tokens != NULL && tokens[0] != NULL)
    {
      executeGeneralCommand(tokens);
    }
    free(tokens);
  }
}