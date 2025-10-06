#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>

int range = 0;
int account_balance = 0;
sem_t sem;

void* increment(void* arg) {
    int start = *(int*) arg;
    int end = start + range - 1;
    sem_wait(&sem);
    for (int i = start; i <= end; i++) account_balance++;
    sem_post(&sem);
    return NULL;
}

int main(int argc, char* argv[]) {
    struct timeval begin, end;
    int no_of_threads = atoi(argv[1]);
    int money = 2048;
    range = money / no_of_threads;
    pthread_t threads[no_of_threads];
    int start[no_of_threads];
    sem_init(&sem, 0, 1);
    gettimeofday(&begin, NULL);
    for (int i = 0; i < no_of_threads; i++) {
        start[i] = i * range + 1;
        pthread_create(&threads[i], NULL, increment, &start[i]);
    }
    for (int i = 0; i < no_of_threads; i++) pthread_join(threads[i], NULL);
    gettimeofday(&end, NULL);
    int64_t seconds = end.tv_sec - begin.tv_sec;
    int64_t microseconds = end.tv_usec - begin.tv_usec;
    double time = seconds + microseconds * 1e-6;
    printf("Time taken by %d threads is %f\n", no_of_threads, time);
    printf("Account balance is %d\n", account_balance);
    return 0;
}
