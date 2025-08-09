#include <stdio.h>
#include <unistd.h>

void create_processes(int level) {
    if (level == 0) return;
    pid_t left_child = fork();
    if (left_child == 0) create_processes(level - 1);
    else if (left_child != 0) {
        pid_t right_child = fork();
        if (right_child == 0) create_processes(level - 1);
    }
}

int main (void) {
    pid_t root_process_id = getpid();
    pid_t process_group_id = getpgrp();
    printf("HERE : %d\n", root_process_id);
    create_processes(3);
    sleep(10000);
}
