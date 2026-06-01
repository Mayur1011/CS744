#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "eval.h"
#include "helper.h"
#include "redirection.h"

int backgroundJobID = 1;

typedef struct BackgroundJob
{
  int jobID;
  pid_t pid;
  char **commandTokens;
} BackgroundJob;

typedef struct BackgroundJobNode
{
  BackgroundJob job;
  struct BackgroundJobNode *next;
  struct BackgroundJobNode *prev;
} BackgroundJobNode;

struct BackgroundJobList
{
  BackgroundJobNode *head;
  BackgroundJobNode *tail;
  int currentJobID;
  int previousJobID;
} bgJobs;

void initializeBackgroundJobsList()
{
  bgJobs.head = NULL;
  bgJobs.tail = NULL;
  bgJobs.currentJobID = 0;
  bgJobs.previousJobID = 0;
}

void printJobCompletionMessage(pid_t childProcessId)
{
  BackgroundJobNode *currentNode = bgJobs.head;
  while (currentNode != NULL)
  {
    if (currentNode->job.pid == childProcessId)
    {
      printf("[%d] Done            ", currentNode->job.jobID);
      for (int i = 0; currentNode->job.commandTokens[i] != NULL; i++)
      {
        printf("%s ", currentNode->job.commandTokens[i]);
      }
      printf("\n");
      break;
    }
    currentNode = currentNode->next;
  }
}

void updateBackgroundJobsList(pid_t childProcessId)
{
  BackgroundJobNode *currentNode = bgJobs.head;
  while (currentNode != NULL && currentNode->job.pid != childProcessId)
  {
    currentNode = currentNode->next;
  }
  if (currentNode != NULL)
  {
    if (currentNode->prev == NULL)
    {
      // Removing the head node
      bgJobs.head = currentNode->next;
      if (bgJobs.head != NULL)
        bgJobs.head->prev = NULL;
    }
    else if (currentNode->next == NULL)
    {
      // Removing the tail node
      bgJobs.tail = currentNode->prev;
      if (bgJobs.tail != NULL)
        bgJobs.tail->next = NULL;
    }
    else
    {
      // Removing a middle node
      currentNode->prev->next = currentNode->next;
      currentNode->next->prev = currentNode->prev;
    }
  }
  freeTokens(currentNode->job.commandTokens, getNoOfTokens(currentNode->job.commandTokens));
  free(currentNode);
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

void executeCatCommand(const char *command)
{
  int tokenCount = 0;
  char **tokens = splitCommandIntoTokens(command, &tokenCount);

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

  freeTokens(tokens, tokenCount);
}

void executeGeneralCommand(char **tokens)
{
  pid_t childPid = fork();
  if (childPid == 0)
  {
    if (execvp(tokens[0], tokens) == -1)
    {
      printCommandNotFoundError(tokens[0]);
      exit(1);
    }
  }
  else
  {
    waitpid(childPid, NULL, 0);
  }
}

void addJobToBackgroundJobsList(int jobID, pid_t pid, char **commandTokens)
{
  BackgroundJob newJob;
  newJob.jobID = jobID;
  newJob.pid = pid;
  int numTokens = getNoOfTokens(commandTokens);
  newJob.commandTokens = malloc((numTokens + 1) * sizeof(char *));
  for (int i = 0; i < numTokens; i++)
  {
    newJob.commandTokens[i] = strdup(commandTokens[i]);
  }
  newJob.commandTokens[numTokens] = NULL;

  BackgroundJobNode *newNode = malloc(sizeof(BackgroundJobNode));
  newNode->job = newJob;
  newNode->next = NULL;
  newNode->prev = NULL;

  if (bgJobs.head == NULL)
  {
    bgJobs.head = newNode;
    bgJobs.tail = newNode;
  }
  else
  {
    bgJobs.tail->next = newNode;
    newNode->prev = bgJobs.tail;
    bgJobs.tail = newNode;
  }
  bgJobs.previousJobID = bgJobs.currentJobID;
  bgJobs.currentJobID = jobID;
}

void executeBackgroundCommand(char **tokens)
{
  // printf("Executing background command: %s\n", tokens[0]);
  pid_t childPid = fork();
  if (childPid == 0)
  {
    if (execvp(tokens[0], tokens) == -1)
    {
      printf("Background job [%d] failed\n", backgroundJobID);
      exit(1);
    }
  }
  else
  {
    printf("[%d] %d\n", backgroundJobID, childPid);
    addJobToBackgroundJobsList(backgroundJobID, childPid, tokens);
    backgroundJobID++;
  }
}

void evalCommand(const char *command)
{
  char **globalTokens = splitCommandIntoTokens(command, &(int){0});
  int globalTokensCount = getNoOfTokens(globalTokens);

  if (strncmp(command, "exit", 4) == 0)
    executeExitCommand();
  else if (checkRedirectionMode(globalTokens) == 1)
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    char *outputFile = NULL;
    int tokenNo = 0;
    while (tokens[tokenNo] != NULL)
    {
      if (strcmp(tokens[tokenNo], ">") == 0 || strcmp(tokens[tokenNo], "1>") == 0)
      {
        outputFile = tokens[tokenNo + 1];
        free(tokens[tokenNo]);
        tokens[tokenNo] = NULL; // command before > or 1>
        break;
      }
      tokenNo++;
    }
    if (outputFile == NULL)
    {
      fprintf(stderr, "Error: output file not specified for output redirection\n");
      freeTokens(tokens, globalTokensCount);
      freeTokens(globalTokens, globalTokensCount);
      return;
    }

    executeStdoutRedirection(tokens, outputFile);
    freeTokens(tokens, globalTokensCount);
  }
  else if (checkAppendRedirectionMode(globalTokens) == 1)
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    char *outputFile = NULL;
    int tokenNo = 0;
    while (tokens[tokenNo] != NULL)
    {
      if (strcmp(tokens[tokenNo], ">>") == 0 || strcmp(tokens[tokenNo], "1>>") == 0)
      {
        outputFile = tokens[tokenNo + 1];
        free(tokens[tokenNo]);
        tokens[tokenNo] = NULL; // command before >> or 1>>
        break;
      }
      tokenNo++;
    }
    if (outputFile == NULL)
    {
      fprintf(stderr, "Error: output file not specified for output redirection\n");
      freeTokens(tokens, globalTokensCount);
      freeTokens(globalTokens, globalTokensCount);
      return;
    }
    executeStdoutAppend(tokens, outputFile);
    freeTokens(tokens, globalTokensCount);
  }
  else if (globalTokensCount > 1 && strcmp(globalTokens[globalTokensCount - 1], "&") == 0)
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    free(tokens[globalTokensCount - 1]);
    tokens[globalTokensCount - 1] = NULL; // command before &
    executeBackgroundCommand(tokens);
    freeTokens(tokens, globalTokensCount);
  }
  else if (strcmp(globalTokens[0], "cd") == 0)
  {
    if (globalTokens[1] == NULL)
    {
      const char *homeDir = getenv("HOME");
      if (chdir(homeDir) != 0)
      {
        fprintf(stderr, "cd: %s: No such file or directory\n", homeDir);
      }
    }
    else
    {
      if (chdir(globalTokens[1]) != 0)
      {
        fprintf(stderr, "cd: %s: No such file or directory\n", globalTokens[1]);
      }
    }
  }
  else if (strncmp(command, "cat", 3) == 0)
    executeCatCommand(command);
  else if (strncmp(command, "echo", 4) == 0)
  {
    // TODO: read about sscanf
    // sscanf(command, "echo %[^\n]", command);

    executeEchoCommand(command);
  }
  else if (strcmp(globalTokens[0], "jobs") == 0)
  {
    BackgroundJobNode *currentNode = bgJobs.head;
    while (currentNode != NULL)
    {
      printf("[%d]", currentNode->job.jobID);
      if (currentNode->job.jobID == bgJobs.currentJobID)
        printf("+ ");
      else if (currentNode->job.jobID == bgJobs.previousJobID)
        printf("- ");
      else
        printf("  ");

      printf("Running          ");

      for (int i = 0; currentNode->job.commandTokens[i] != NULL; i++)
      {
        printf("%s ", currentNode->job.commandTokens[i]);
      }
      printf("&\n");

      currentNode = currentNode->next;
    }
  }
  else
  {
    char **tokens = splitCommandIntoTokens(command, &(int){0});
    if (tokens != NULL && tokens[0] != NULL)
    {
      executeGeneralCommand(tokens);
    }
    freeTokens(tokens, globalTokensCount);
  }
  freeTokens(globalTokens, globalTokensCount);
}