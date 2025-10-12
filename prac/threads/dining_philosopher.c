#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define EATING 1
#define HUNGRY 0
#define LEFT(i) (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS
#define RIGHT(i) (i + 1) % NUM_PHILOSOPHERS

pthread_mutex_t mutex;
pthread_cond_t cond[NUM_PHILOSOPHERS];
int state[NUM_PHILOSOPHERS];

int can_take_forks(int philosopher_id) {
    if (state[philosopher_id] == HUNGRY && state[LEFT(philosopher_id)] != EATING && state[RIGHT(philosopher_id)] != EATING) {
        return 1;
    }
    return 0;
}

void put_forks(int philosopher_id) {
    state[philosopher_id] = HUNGRY;
    if(can_take_forks(LEFT(philosopher_id))) pthread_cond_signal(&cond[LEFT(philosopher_id)]);
    if(can_take_forks(RIGHT(philosopher_id))) pthread_cond_signal(&cond[RIGHT(philosopher_id)]);
}

void* handle_philosopher(void* arg) {
    int philosopher_id = *(int*)arg;
    while(1) {
        pthread_mutex_lock(&mutex);
        printf("Philosopher %d is thinking.\n", philosopher_id);
        sleep(1);
        while(can_take_forks(philosopher_id) == 0) { pthread_cond_wait(&cond[philosopher_id], &mutex); }
        state[philosopher_id] = EATING;
        pthread_mutex_unlock(&mutex);
        printf("philosopher %d is eating.\n", philosopher_id);
        sleep(1);
        put_forks(philosopher_id);
    }
    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int philosopher_ids[NUM_PHILOSOPHERS];
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        state[i] = HUNGRY;
        pthread_cond_init(&cond[i], NULL);
    }

    for(int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, handle_philosopher, &philosopher_ids[i]);
    }

    for(int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    return 0;
}

