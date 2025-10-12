#include "rwlock.h"
#include <pthread.h>

void InitalizeReadWriteLock(struct read_write_lock *rw)
{
    //	Write the code for initializing your read-write lock.
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->can_read, NULL);
    pthread_cond_init(&rw->can_write, NULL);
    rw->reader_count = 0;
    rw->writer_count = 0;
    rw->writer_writing = false;
}

void ReaderLock(struct read_write_lock *rw)
{
    //	Write the code for aquiring read-write lock by the reader.
    pthread_mutex_lock(&rw->mutex);
    while (rw->writer_count > 0 || rw->writer_writing)
        pthread_cond_wait(&rw->can_read, &rw->mutex);
    rw->reader_count++;
    pthread_mutex_unlock(&rw->mutex);
}

void ReaderUnlock(struct read_write_lock *rw)
{
    //	Write the code for releasing read-write lock by the reader.
    pthread_mutex_lock(&rw->mutex);
    rw->reader_count--;
    if (rw->reader_count == 0)
        pthread_cond_signal(&rw->can_write);
    pthread_mutex_unlock(&rw->mutex);
}

void WriterLock(struct read_write_lock *rw)
{
    //	Write the code for aquiring read-write lock by the writer.
    pthread_mutex_lock(&rw->mutex);
    rw->writer_count++;
    while (rw->reader_count > 0 || rw->writer_writing)
        pthread_cond_wait(&rw->can_write, &rw->mutex);
    rw->writer_writing = true;
    pthread_mutex_unlock(&rw->mutex);
}

void WriterUnlock(struct read_write_lock *rw)
{
    //	Write the code for releasing read-write lock by the writer.
    pthread_mutex_lock(&rw->mutex);
    rw->writer_writing = false;
    rw->writer_count--;
    if (rw->writer_count == 0)
        pthread_cond_broadcast(&rw->can_read);
    else
        pthread_cond_signal(&rw->can_write);
    pthread_mutex_unlock(&rw->mutex);
}
