#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <sys/wait.h>

#include "eval.h"
#include "helper.h"
#include "completion.h"
#include "redirection.h"

char *PATH_ENV_VAR = "/usr/local/bin:/usr/bin";

int main(int argc, char *argv[])
{
  setbuf(stdout, NULL); // Flush after every printf

  // tab completion callback function
  rl_attempted_completion_function = commandCompletion;

  char *command = NULL;
  initializeBackgroundJobsList();

  while (1)
  {
    while (1)
    {
      int childProcessId = waitpid(-1, NULL, WNOHANG);
      if (childProcessId <= 0)
        break;
      printJobCompletionMessage(childProcessId);
      updateBackgroundJobsList(childProcessId);
    }
    char *currentDir = getcwd(NULL, 0);
    if (currentDir != NULL)
    {
      char prompt[1024];
      snprintf(prompt, sizeof(prompt), "%s $ ", currentDir);
      free(currentDir);
      command = readline(prompt);
    }
    else
      command = readline("$ ");

    while (1)
    {
      int childProcessId = waitpid(-1, NULL, WNOHANG);
      if (childProcessId <= 0)
        break;
      printJobCompletionMessage(childProcessId);
      updateBackgroundJobsList(childProcessId);
    }

    if (command == NULL)
      break;

    if (command[0] == '\0')
    {
      free(command);
      continue;
    }

    // add_history(command);
    evalCommand(command);
    free(command);
  }
  return 0;
}
