#include <stdio.h>
#include <unistd.h>

int main()
{
    int a = 10;
    if (fork() == 0)
    {
        a += 5;
        printf("Child: %d at %p\n", a, (void *)&a);
    }
    else
    {
        a -= 5;
        printf("Parent: %d at %p\n", a, (void *)&a);
    }
    return 0;
}