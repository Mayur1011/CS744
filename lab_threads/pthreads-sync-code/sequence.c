#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define N 3
pthread_cond_t cond;
pthread_mutex_t lock;
int turn = 0;

void *print_thread_message(void *arg)
{
    int thread_num = *(int *)arg;
    pthread_mutex_lock(&lock);

    while (1)
    {
        while (turn != thread_num)
            pthread_cond_wait(&cond, &lock);

        printf("I am thread %d\n", thread_num);
        fflush(stdout);

        turn = (turn + 1) % N;
        pthread_cond_broadcast(&cond);
    }

    pthread_mutex_unlock(&lock);
    return NULL;
}

int main()
{

    pthread_t threads[N];
    int thread_nums[N];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < N; i++)
        thread_nums[i] = i;

    // Create N threads
    for (int i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, print_thread_message, &thread_nums[i]);

    // Wait for all threads (though in this case, the threads run forever)
    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
