#include <pthread.h>
#include <stdio.h>

int counter = 0;
pthread_mutex_t lock;

void* increment_counter(void* arg) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < 1000; i++) counter++;
    pthread_mutex_unlock(&lock);
    return NULL;
}


int main() {
    pthread_t thread[10];
    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < 10; i++) {
        pthread_create(&thread[i], NULL, increment_counter, NULL);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(thread[i], NULL);
    }
    
    printf("The final value of counter is: %d\n", counter);
    return 0;
}
