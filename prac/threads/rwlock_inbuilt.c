#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int shared_variable = 0;
pthread_rwlock_t rwlock;

void *reader(void *arg)
{
    int reader_id = *(int *)arg;
    pthread_rwlock_rdlock(&rwlock);
    printf("Reader %d has started reading.\n", reader_id);
    sleep(1);
    printf("Reader %d read shared_variable = %d\n", reader_id, shared_variable);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *writer(void *arg)
{
    int writer_id = *(int *)arg;
    pthread_rwlock_wrlock(&rwlock);
    printf("Writer %d has stared writing.\n", writer_id);
    shared_variable += 1;
    sleep(1);
    printf("Writer %d updated shared_variable to %d\n", writer_id, shared_variable);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

int main()
{
    pthread_t reader_t[3], writer_t[2];
    pthread_rwlock_init(&rwlock, NULL);
    int reader_id[3], writer_id[2];
    for (int i = 0; i < 2; i++)
    {
        writer_id[i] = i;
        pthread_create(&writer_t[i], NULL, writer, &writer_id[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        reader_id[i] = i;
        pthread_create(&reader_t[i], NULL, reader, &reader_id[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        pthread_join(reader_t[i], NULL);
    }
    for (int i = 0; i < 2; i++)
    {
        pthread_join(writer_t[i], NULL);
    }
    return 0;
}
