#include <stdio.h>


int main() {
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
}
