#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

int counter = 0;
sem_t lock;

void* increment_counter(void* arg) {
    sem_wait(&lock);
    for (int i = 0; i < 1000; i++) counter++;
    sem_post(&lock);
    return NULL;
}

int main() {
    pthread_t thread[10];
    sem_init(&lock, 0, 1);

    for (int i = 0; i < 10; i++) {
        pthread_create(&thread[i], NULL, increment_counter, NULL);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(thread[i], NULL);
    }

    printf("The final value of counter is : %d\n", counter);

    return 0;
} 
