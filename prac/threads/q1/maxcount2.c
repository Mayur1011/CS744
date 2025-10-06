#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_NUM 10000

// some variables, most likely will need more
int num_threads1;
int num_threads2;
int percentile;            
int global_max = 0;
int global_count = 0;
int threshold;

// Structure for each thread arguments
typedef struct
{
    int thread_id;
    int *buffer;
    int start;
    int end;
    int local_max;
    int local_count;
} thread_args_t;


pthread_mutex_t lock;
pthread_cond_t part1;

// Function to open a file and store its content into an array
int read_file(const char *filename, int **buffer) {

    /* opening file */
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
    /* Copying numbers from file to buffer array */

    *buffer = malloc(MAX_NUM * sizeof(int));
    int count = 0;

    while (fscanf(file, "%d", &(*buffer)[count]) == 1) {
        count++;
    }

    fclose(file);
    return count;
}

// task done by threads of thread pool 1

int threads_done = 0;
void *global_max_task(void *arg)
{
    /* TODO 5: get thread arguments */
    thread_args_t* args = (thread_args_t*)arg;

    pthread_mutex_lock(&lock);
    printf("[POOL1-THREAD %d] waiting at barrier\n", args->thread_id);
    pthread_cond_wait(&part1, &lock);
    printf("[POOL1-THREAD %d] crossed the barrier\n", args->thread_id);
    pthread_mutex_unlock(&lock);

    /* TODO 6: Find the local max in the assigned chunk */
    for (int i = args->start; i <= args->end; i++) {
        if (args->buffer[i] > args->local_max) {
            args->local_max = args->buffer[i];
        }
    } 

    printf("[POOL1-THREAD %d] Start = %d End = %d Local max = %d\n",
           args->thread_id, args->start, args->end, args->local_max);

    // TODO 7: Compute the global maximum and threshold 
    pthread_mutex_lock(&lock);
    if (args->local_max > global_max) {
        global_max = args->local_max;
        threshold = ((double)percentile / 100) * global_max;
    }
    threads_done++;
    // start: Do not touch 
    if (threads_done == num_threads1) {
        printf("------------------------------------------------------------\n");
        printf("[POOL1] Global max = %d\n", global_max);
        printf("[POOL1] Threshold = %d\n", threshold);
        printf("[POOL1]: All threads done. \n");
        printf("------------------------------------------------------------\n");
    }
    pthread_mutex_unlock(&lock);
    // end: Do not touch

    // TODO 8: Work of thread in thead pool 1 almost done 

    return NULL;
}

// Done by threads of thread pool 2
void *global_count_task(void *arg)
{
    /* TODO 9: get thread arguments */
    thread_args_t* args = (thread_args_t*)arg;


    // TODO 10: calculate the number of integers which are greater than threshold

    printf("[POOL2-THREAD %d] Start = %d End = %d Local count = %d\n",
           args->thread_id, args->start, args->end, args->local_count);

    // TODO 11: calculate global count 

    // start: Do not touch
    printf("------------------------------------------------------------\n");
    printf("[POOL2] Global count = %d\n", global_count);
    printf("[POOL2]: All threads done.\n");
    // end: Do not touch

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <first-file> <second-file> <tcnt1> <tcnt2> <percentile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename1 = argv[1];
    char *filename2 = argv[2];
    num_threads1 = atoi(argv[3]);
    num_threads2 = atoi(argv[4]);
    percentile = atoi(argv[5]);

    /* TODO 1: Initialize the thread and synchronization variables */
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&part1, NULL);

    int *file1_buffer, *file2_buffer;
    int file1_count = read_file(filename1, &file1_buffer);
    if (file1_count == -1) {
        free(file1_buffer);
        return EXIT_FAILURE;
    }

    int file2_count = read_file(filename2, &file2_buffer);
    if (file2_count == -1) {
        free(file2_buffer);
        return EXIT_FAILURE;
    }

    int chunk_size = file1_count / num_threads1;
    printf("[MAIN] Pool1 chunksize = %d\n", chunk_size);

    /* TODO 2: Initialize arguments and create threads of thread pool 1 */
    pthread_t POOL1[num_threads1];
    thread_args_t args1[num_threads1];
    for (int i = 0; i < num_threads1; i++)
    {
        args1[i].thread_id = POOL1[i];
        args1[i].buffer = file1_buffer;
        args1[i].start = i * chunk_size;
        args1[i].end = (i == num_threads1 - 1) ? file1_count - 1 : args1[i].start + chunk_size - 1;
        args1[i].local_count = 0;
        args1[i].local_max = 0;
        pthread_create(&POOL1[i], NULL, global_max_task, &args1[i]);
    }
    usleep(500);
    pthread_cond_broadcast(&part1);
    printf("[POOL1] Threads created.\n");

    for (int i = 0; i < num_threads1; i++) pthread_join(POOL1[i], NULL);

    chunk_size = file2_count / num_threads2;
    printf("[MAIN] Pool2 chunksize = %d\n", chunk_size);

    /* TODO 3: Initialize arguments and create threads of thread pool 2 */
    for (int i = 0; i < num_threads2; i++)
    {



    }

    // start: Do not touch
    printf("[POOL2] Threads created.\n");
    printf("------------------------------------------------------------\n");
    // end: Do not touch

    /* TODO 4: All threads of both thread pools should be created before this point 
   of execution
   Continue execution of required logic as specified in question  */


    // TODO 12: Wait for all threads to finish


    // TODO 13: clean up    
    free(file1_buffer);
    free(file2_buffer);


    return 0;
}
