#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char input[50];
    int n;

    while (1)
    {
        printf(">>> ");

        /*PARSING LOGIC START*/
        scanf("%s", input);
        scanf("%d", &n);

        char **args = (char **)malloc((n + 3) * sizeof(char *));

        args[0] = input;
        args[1] = (char *)malloc(10 * sizeof(char));
        sprintf(args[1], "%d", n);
        for (int i = 2; i <= n + 1; i++)
        {
            args[i] = (char *)malloc(10 * sizeof(char));
            scanf("%s", args[i]);
        }
        args[n + 2] = NULL;
        /*PARSING LOGIC END*/

        /*****YOU MAY WRITE YOUR LOGIC HERE***********/

        /*********************************************/

        free(args);
    }
    return 0;
}
