#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc != 3) { printf("Invalid input");}
    execlp(argv[1], argv[1], argv[2], NULL);

    printf("This should not exists\n");

    return 0;
}
