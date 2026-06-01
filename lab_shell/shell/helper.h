#ifndef HELPER_H
#define HELPER_H

char *PATH_ENV_VAR = "/usr/local/bin:/usr/bin";

char **splitCommandIntoTokens(const char *command, int *numTokens);
void printCommandNotFoundError(const char *command);

#endif