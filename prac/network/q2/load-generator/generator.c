#include "http_client.h"
FILE *log_file;
int done;
float time_diff(struct timeval *t1, struct timeval *t2)
{
    return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}
void *user_routine(void *args)
{
    struct user_info *info = (struct user_info *)args;
    int sockfd, n;
    char buffer[256];
    struct timeval start, end;

    struct hostent *server;
    struct sockaddr_in serv_addr;

    // printf("Host name: %s, Port: %d\n", info->hostname, info->portno);

    server = gethostbyname(info->hostname);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    /* TODO: 4. Setup port number */
    serv_addr.sin_port = htons(info->portno);

    while (1)
    {
        /* start timer */
        gettimeofday(&start, NULL);

        /* TODO: 5. create the socket to send a request and get a reply */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Error opening socket");
            exit(EXIT_FAILURE);
        }

        /* TODO: 6. connect to the server */
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Connection to server failed");
            exit(EXIT_FAILURE);
        }

        /* generating buffer with HTTP request (to send) */
        struct HTTP_Request request;
        init_http_request(&request, URL);
        get_request_string(&request, buffer);

        /* TODO: 7. send request to the server */
        gettimeofday(&start, NULL);
        n = send(sockfd, buffer, strlen(buffer), 0);
        if (n < 0)
        {
            perror("Error writing to socket");
            exit(EXIT_FAILURE);
        }

        /* TODO: 8. read response from the server */
        int bytes = 0;
        char response[1024] = {0};
        while (1)
        {
            bzero(buffer, sizeof(buffer));
            n = recv(sockfd, buffer, 255, 0);
            if (n == 0)
                break;
            if (n < 0)
            {
                perror("Error reading from server");
                exit(EXIT_FAILURE);
            }
            strcat(response, buffer);
            bytes += n;
        }

        /* TODO: 9. close the socket */
        close(sockfd);

        /* TODO: 10. stop timer and caclulate elapsed_time, you need to use it for updating user metrics */
        gettimeofday(&end, NULL);
        float elapsed_time = time_diff(&start, &end);

        /* if test_duration is up, break */
        if (done)
        {
            /* TODO: 11. Print thread requests and byte count */
            printf("Thread sent %d requests. Total bytes received: %d\n", info->total_count, bytes);
            break;
        }
        /* TODO: 12. update user metrics */
        info->total_rtt += elapsed_time;
        info->total_count++;

        /* TODO: 13. use usleep for think time  */
        usleep(info->think_time * 1000);
    }

    fprintf(log_file, "Thread #%d finished\n", info->user_id);
    fflush(log_file);
    /* TODO: 14. exit thread */
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int test_duration, user_count;
    float think_time, throughput = 0, response_time = 0;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <user_count> <think_time (ms)> <test_duration (s)>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    user_count = atoi(argv[1]);
    think_time = atof(argv[2]);
    test_duration = atoi(argv[3]);

    printf("Host: %s\nPort: %d\nUser Count: %d\nThink Time: %f ms\nTest Duration: %d s\nTesting ...\n",
           HOST, PORT, user_count, think_time, test_duration);

    /* open log file */
    log_file = fopen("load_gen.log", "w");

    fprintf(log_file, "Host: %s\n", HOST);
    fprintf(log_file, "Port: %d\n", PORT);
    fprintf(log_file, "User Count: %d\n", user_count);
    fprintf(log_file, "Think Time: %f ms\n", think_time);
    fprintf(log_file, "Test Duration: %d s\n", test_duration);

    /* TODO: 1. initialize pthread related variables */
    pthread_t users[user_count];
    struct user_info *info = (struct user_info *)malloc(sizeof(struct user_info) * user_count);
    struct timeval start_time, end_time;

    // starts user threads and the expriment
    gettimeofday(&start_time, NULL);
    done = 0;

    for (int i = 0; i < user_count; i++)
    {
        info[i].hostname = HOST;
        info[i].portno = PORT;
        /* TODO: 2. initialize other user_info (defination in http_client.h) that needs to be passed to the user thread as an argument */
        info[i].user_id = users[i];
        info[i].think_time = think_time;
        info[i].total_count = 0;
        info[i].total_rtt = 0;
        /* TODO: 3. create user thread to simulate a user */
        pthread_create(&users[i], NULL, user_routine, &info[i]);
        fprintf(log_file, "Created Thread %d\n", i);
    }

    /* TODO: 15. use sleep to emulate test duration */
    sleep(test_duration);
    fprintf(log_file, "Main Thread Woke up!!\n");

    /* TODO: 16. signal and stop all threads as test duration has elapsed */
    gettimeofday(&end_time, NULL);
    done = 1;

    /* TODO: 17. wait for all threads to finish */
    for (int i = 0; i < user_count; i++)
    {
        pthread_join(users[i], NULL);
    }

    /* TODO: 18. calculate results */
    int total_requests = 0;
    float total_response_time = 0;
    for (int i = 0; i < user_count; i++)
    {
        total_requests += info[i].total_count;
        total_response_time += info[i].total_rtt;
    }
    throughput = (float)total_requests / test_duration;
    response_time = total_response_time / total_requests;

    /* TODO: 19. print total number of requests and total response time */
    fprintf(log_file, "\n\nAverage Throughput: %f req/s\n", throughput);
    fprintf(log_file, "Average Response Time: %f ms\n", response_time * 1000);

    printf("\n\nTotal requests: %d\nTotal response time: %f\nAverage Throughput: %f req/s\nAverage Response Time: %f ms\n", total_requests, total_response_time, throughput, response_time * 1000);

    /* close log file */
    fclose(log_file);
    free(info);

    return 0;
}
