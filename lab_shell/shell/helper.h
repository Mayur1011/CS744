#ifndef HELPER_H
#define HELPER_H

extern char *PATH_ENV_VAR;

char **splitCommandIntoTokens(const char *command, int *numTokens);
int getNoOfTokens(char **tokens);
void freeTokens(char **tokens, int noOfTokens);
void printCommandNotFoundError(const char *command);

#endif