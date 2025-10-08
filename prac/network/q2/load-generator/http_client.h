#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

// Configuration
#define HOST "localhost"
#define URL "/index.html"
#define PORT 8080
#define SANITY_CHECK 0
#define OUTPUT 0
#define FAULT_EXIT 0

struct HTTP_Request
{
    char HTTP_version[8];
    char method[8];
    char url[256];
};

// user info struct to pass to thread
struct user_info
{
    // user id
    int user_id; // to identify the thread

    // socket info
    int portno;
    char *hostname;
    float think_time;

    // user metrics
    int total_count; // total requests
    float total_rtt; // total response time
};

void get_request_string(struct HTTP_Request *request, char *buffer);
void init_http_request(struct HTTP_Request *request, const char *url);

#endif
