#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char message[342];
    int x = 123;
    snprintf(message, sizeof(message), "Hello, World! %d\n", x);
    printf("%s", message);

    char str[100];
    sscanf(message, "Hello, %s", str);
    x = 23131;
    sscanf(message, "Hello, World! %d", &x);
    printf("%s\n", str);
    printf("%s\n", message);

    // char *dir = "/home";
    // char *file = "mayur";
    // char *path = malloc(strlen(dir) + strlen(file) + 2);
    // snprintf(path, strlen(dir) + strlen(file) + 2, "%s/%s", dir, file);
    // printf("%s\n", path);
    // free(path);
    return 0;
}
