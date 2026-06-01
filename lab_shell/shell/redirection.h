#ifndef REDIRECTION_H
#define REDIRECTION_H

int checkRedirectionMode(char **tokens);
int checkAppendRedirectionMode(char **tokens);
void executeStdoutAppend(char **tokens, const char *outputFile);
void executeStdoutRedirection(char **tokens, const char *outputFile);

#endif