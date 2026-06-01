#ifndef EVAL_H
#define EVAL_H

void initializeBackgroundJobsList();
void printJobCompletionMessage(pid_t childProcessId);
void updateBackgroundJobsList(pid_t childProcessId);
void evalCommand(const char *command);

#endif