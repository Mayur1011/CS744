#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"

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

int getNoOfTokens(char **tokens)
{
  int count = 0;
  while (tokens[count] != NULL)
    count++;
  return count;
}

void freeTokens(char **tokens, int noOfTokens)
{
  for (int i = 0; i < noOfTokens; i++)
    free(tokens[i]);
  free(tokens);
}

void printCommandNotFoundError(const char *command)
{
  fprintf(stderr, "%s: command not found\n", command);
}