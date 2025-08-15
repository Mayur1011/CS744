#include <stdio.h>
#include <unistd.h>

int main()
{
	printf("PID: %d, PPID: %d\n", getpid(), getppid());
	printf("Hello, World!\n");
	return 0;
}
