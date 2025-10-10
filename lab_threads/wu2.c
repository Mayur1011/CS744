#include <stdio.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t cond;
int turn = 0;

void *print_thread(void *arg)
{
    int id = *((int *)arg);
    pthread_mutex_lock(&lock);
    while (turn != id)
    {
        pthread_cond_wait(&cond, &lock);
    }
    printf("Hello from thread %d\n", id);
    turn++;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main()
{
    pthread_t thread[10];
    int value[10];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < 10; i++)
    {
        value[i] = i;
        pthread_create(&thread[i], NULL, print_thread, (void *)&value[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        pthread_join(thread[i], NULL);
    }

    printf("I am the main thread\n");
    return 0;
}
