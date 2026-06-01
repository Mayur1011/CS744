#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "helper.h"
#include "redirection.h"

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
      printCommandNotFoundError(tokens[0]);
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
      printCommandNotFoundError(tokens[0]);
      exit(1);
    }
  }
  else if (childPid > 0)
  {
    waitpid(childPid, NULL, 0);
  }
  return;
}