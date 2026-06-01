#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <readline/readline.h>
#include <sys/stat.h>

#include "completion.h"
#include "helper.h"

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
char *completeFilename(const char *text, int state)
{
  static int filePrefixLen = 0;
  static char *dirPath = NULL;
  static DIR *dir = NULL;
  static char *filePrefix = NULL;

  if (state == 0)
  {
    dirPath = NULL;
    if (dir)
      closedir(dir);
    filePrefix = NULL;

    // Find the last occurrence of /
    char *lastSlash = strrchr(text, '/');
    if (lastSlash != NULL)
    {
      int dirPathLen = lastSlash - text + 1;
      dirPath = malloc(dirPathLen + 1);
      strncpy(dirPath, text, dirPathLen);
      dirPath[dirPathLen] = '\0';
      filePrefix = strdup(lastSlash + 1);

      dir = opendir(dirPath);
    }
    else
    {
      dirPath = NULL;
      filePrefix = strdup(text);

      dir = opendir(".");
    }

    filePrefixLen = strlen(filePrefix);
  }

  if (dir == NULL)
    return NULL;

  struct dirent *currentFile;
  while ((currentFile = readdir(dir)) != NULL)
  {
    if (strncmp(currentFile->d_name, filePrefix, filePrefixLen) == 0)
    {
      if (strcmp(currentFile->d_name, ".") == 0 || strcmp(currentFile->d_name, "..") == 0)
        continue;

      char *fileName = NULL;
      if (dirPath != NULL)
      {
        fileName = malloc(strlen(dirPath) + strlen(currentFile->d_name) + 1);
        sprintf(fileName, "%s%s", dirPath, currentFile->d_name);
      }
      else
      {
        fileName = strdup(currentFile->d_name);
      }

      struct stat fileStat;
      if (stat(fileName, &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
        rl_completion_append_character = '/';
      else
        rl_completion_append_character = ' ';

      return fileName;
    }
  }

  closedir(dir);
  dir = NULL;
  return NULL;
}

char **commandCompletion(const char *text, int start, int end)
{
  rl_attempted_completion_over = 1; // don't use default filename completion

  // return rl_completion_matches(text, completeCommandExecutable);

  // If you want tab completion for first word
  if (start == 0)
    return rl_completion_matches(text, completeCommandExecutable);
  else
    return rl_completion_matches(text, completeFilename);
}