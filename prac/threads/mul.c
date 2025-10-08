#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
int N, M, O;
int** res = NULL;
int** arr1 = NULL;
int** arr2 = NULL;
pthread_mutex_t lock;
void* multiply(void* arg) {
    int row = *(int*)arg;
    int curr_ans = 0;
    for (int i = 0; i < O; i++) {
        for (int j = 0; j < M; j++) {
            curr_ans += arr1[row][j] * arr2[j][i];
        }
        res[row][i] = curr_ans;
        curr_ans = 0;
    }
    return NULL;
}
int main() {
    scanf("%d%d%d", &N, &M, &O);

    arr1 = (int**) malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        arr1[i] = (int*) malloc(M * sizeof(int));
        for (int j = 0; j < M; j++) {
            scanf("%d", &arr1[i][j]);
        }
    }
    arr2 = (int**) malloc(M * sizeof(int*));
    for (int i = 0; i < M; i++) {
        arr2[i] = (int*) malloc(O * sizeof(int));
        for (int j = 0; j < O; j++)
            scanf("%d", &arr2[i][j]);
    }
    res = (int**) malloc (N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        res[i] = (int*) malloc(O * sizeof(int));
    }
    pthread_t thread_pool[N];
    int thread_arg[N];
    pthread_mutex_init(&lock, NULL);
    for (int i = 0; i < N; i++) {
        thread_arg[i] = i;
        pthread_create(&thread_pool[i], NULL, multiply, &thread_arg[i]);
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            printf("%d ", res[i][j]);
        }
        printf("\n");
    }
}
