#include <semaphore.h>
#include <stdio.h>
#define N 5
int philosophers_state[N];
sem_t mutex[N];
int main() {
    for (int i = 0; i < N; i++) {
        sem_init(&mutex[i], 0, 0);
    }
}

