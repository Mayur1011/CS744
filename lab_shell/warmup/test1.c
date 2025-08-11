/* bin_tree_fork.c
 * Build a perfect binary tree of processes to given depth.
 * Usage: ./bin_tree_fork <depth>
 *
 * Each process prints: PID, PPID, and its level in the tree.
 * Parent waits for both children to avoid zombies.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* print and flush so output from different processes is visible */
static void info_print(int level) {
    printf("PID %d (PPID %d) at level %d\n", getpid(), getppid(), level);
    fflush(stdout);
}

void make_tree(int depth, int level) {
    pid_t left, right;
    info_print(level);

    if (depth <= 0) {
        /* leaf node - keep alive for a short while so you can inspect it if desired */
        sleep(1);
        return;
    }

    /* fork left child */
    left = fork();
    if (left < 0) {
        perror("fork (left)");
        return;
    }
    if (left == 0) {
        /* in left child */
        make_tree(depth - 1, level + 1);
        _exit(0); /* ensure child exits after finishing recursion */
    }

    /* fork right child */
    right = fork();
    if (right < 0) {
        perror("fork (right)");
        /* try to reap left child before returning to avoid zombies */
        waitpid(left, NULL, 0);
        return;
    }
    if (right == 0) {
        /* in right child */
        make_tree(depth - 1, level + 1);
        _exit(0);
    }

    /* parent waits for both children */
    int status;
    if (waitpid(left, &status, 0) == -1) {
        if (errno != ECHILD) perror("waitpid (left)");
    }
    if (waitpid(right, &status, 0) == -1) {
        if (errno != ECHILD) perror("waitpid (right)");
    }

    /* keep parent alive briefly so tree is easier to inspect */
    sleep(100);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <depth>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    long depth = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || depth < 0 || depth > 12) {
        /* limit depth to avoid accidental explosion; adjust as needed */
        fprintf(stderr, "Invalid depth. Use a small non-negative integer (e.g. 0..12).\n");
        return EXIT_FAILURE;
    }

    make_tree((int)depth, 0);
    return EXIT_SUCCESS;
}
