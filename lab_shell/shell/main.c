#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>

#include "eval.h"
#include "completion.h"
#include "helper.h"
#include "redirection.h"

int main(int argc, char *argv[])
{
  setbuf(stdout, NULL); // Flush after every printf

  // tab completion callback function
  rl_attempted_completion_function = commandCompletion;

  char *command = NULL;

  while (1)
  {
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
