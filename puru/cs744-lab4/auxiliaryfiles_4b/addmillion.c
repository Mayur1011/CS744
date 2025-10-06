#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

int account_balance = 0;
sem_t lock;

void *increment(void* arg)
{
    sem_wait(&lock);
    for (int i = 0; i < 1000000; i++)
        account_balance++;
    sem_post(&lock);
    return NULL;
}

int main(int argc, char *argv[])
{
    sem_init(&lock, 0, 1);
    int threadNum = 10;
    pthread_t th[threadNum];
    int i;
    for (i = 0; i < threadNum; i++)
    {
        if (pthread_create(th + i, NULL, &increment, NULL) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
        printf("Transaction %d has started\n", i);
    }
    for (i = 0; i < threadNum; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
            return 2;
        printf("Transaction %d has finished\n", i);
    }
    printf("Account Balance is : %d\n", account_balance);
    return 0;
}
