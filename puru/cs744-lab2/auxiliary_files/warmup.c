#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <sys/wait.h>

int main(){
    // char buffer1[10], buffer2[10], buffer3[10];
    // int fd1 = open("warmup.txt", O_RDONLY);
    // int fd2 = open("warmup.txt", O_RDONLY);
    // read(fd1, buffer1, 5);
    // read(fd1, buffer2, 5);
    // read(fd2, buffer3, 10);
    // close(0); // Closing STDIN
    // int fd3 = open("file.txt", O_RDONLY);
    // printf("%d %d %d\n", fd1, fd2, fd3);
    // printf("%s %s %s\n", buffer1, buffer2, buffer3);
    // close(fd1);
    // close(fd2);
    // close(fd3);


    int fd1 = open("abc.txt", O_RDONLY);
    printf("parent before : offset %d\n", (int) lseek(fd1, 0, SEEK_CUR));
    int rc = fork();
    if (rc == 0) {
        rc = lseek(fd1, 10, SEEK_SET);
        printf("child: offset %d\n", rc);
    } else if (rc > 0) {
        int fd2 = open("abc.txt", O_RDONLY);
        (void) wait(NULL);
        printf("parent: offset %d\n", (int) lseek(fd1, 0, SEEK_CUR));
        printf("parent: offset %d\n", (int) lseek(fd2, 0, SEEK_CUR));
    }    
    return 0;
}
