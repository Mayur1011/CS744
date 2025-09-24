#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define N 3
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

void* print_thread_message(void* arg) {
    int thread_num = *(int*)arg;
    pthread_mutex_lock(&lock);

    while (1) {

        // printf("Thread %d is waiting\n", thread_num);
        pthread_cond_wait(thread_num == 0 ? &cond0 : thread_num == 1 ? &cond1 : &cond2, &lock);

        printf("I am thread %d\n", thread_num);
        fflush(stdout);  

        pthread_cond_signal(thread_num == 0 ? &cond1 : thread_num == 1 ? &cond2 : &cond0);
    }

    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {

    pthread_t threads[N];
    int thread_nums[N];

    for(int i = 0; i < N; i++)
        thread_nums[i] = i;

    // Create N threads
    for (int i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, print_thread_message, &thread_nums[i]);
    }
    usleep(500);

    pthread_cond_signal(&cond0);

    // Wait for all threads (though in this case, the threads run forever)
    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
