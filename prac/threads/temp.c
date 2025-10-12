#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    // int x;
    // while (fscanf(fp, "%d", &x) == 1) {
    //     printf("%d\n", x);
    // }

    // char buff[100];
    // while (fgets(buff, sizeof(buff), fp) != NULL)
    // {
    //     // x = atoi(buff);
    //     puts(buff);
    // }

    char *line = NULL;
    size_t len = 0;
    ssize_t bytes_read;

    while ((bytes_read = getline(&line, &len, fp)) != -1)
    {
        // Process the line
        puts(line);
    }

    free(line);
    fclose(fp);
    return 0;
}
