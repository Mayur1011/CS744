#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>

#define maxN 100000
#define MIN(a, b) ((a) < (b) ? (a) : (b))

bool primes[maxN + 1];
int gb_count[maxN / 2 + 1];

void *compute_goldbach(void *arg)
{
    int *endpoints = (int *)arg;
    int start = endpoints[0];
    int end = endpoints[1];

    for (int n = start; n <= end; n += 2)
        for (int p1 = 2; p1 <= n / 2; p1++)
        {
            int p2 = n - p1;
            if (primes[p1] && primes[p2])
                gb_count[n / 2]++;
        }
    return NULL;
}

void multi_threaded_goldback()
{
    pthread_t threads[10];
    int endpoints[10][2];

    int range = maxN / 10;
    if (range & 1)
        range++;
    int start = 4;

    for (int i = 0; i < 10; i++)
    {
        endpoints[i][0] = start;
        endpoints[i][1] = MIN(maxN, start + range);
        pthread_create(&threads[i], NULL, compute_goldbach, (void *)endpoints[i]);
        start += range + 2;
    }

    for (int i = 0; i < 10; i++)
        pthread_join(threads[i], NULL);
}

int main(int argc, char *argv[])
{
    // Sieve of Eratosthenes
    for (int i = 0; i <= maxN; i++)
        primes[i] = true;
    for (int i = 0; i <= maxN / 2; i++)
        gb_count[i] = 0;

    primes[0] = primes[1] = false;

    for (int p = 2; p <= sqrt(maxN); p++)
        if (primes[p])
            for (int i = p * p; i <= maxN; i += p)
                primes[i] = false;

    int prime_count = 0;
    for (int p = 2; p <= maxN; p++)
        if (primes[p])
            prime_count++;
    printf("Computed primes upto %d, count = %d\n", maxN, prime_count);

    // Compute number of Goldbach pairs

    // start timer
    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, 0);

    // THE CALCULATIONS BELOW TO BE DONE IN PARALLEL
    //
    multi_threaded_goldback();

    // THREADS TO JOIN HERE, DO NOT CHANGE CODE BELOW

    // end timer and calculate elapsed time
    gettimeofday(&tv_end, 0);
    unsigned long elapsed_usec = ((tv_end.tv_sec * 1000000) + tv_end.tv_usec) - ((tv_start.tv_sec * 1000000) + tv_start.tv_usec);
    printf("elapsed time: %lu microseconds\n", elapsed_usec);

    // print values to file for checking correctness
    FILE *fptr = fopen("output.txt", "w");
    for (int i = 4; i <= maxN; i += 2)
        fprintf(fptr, "%d %d\n", i, gb_count[i / 2]);
    fclose(fptr);

    return 0;
}
