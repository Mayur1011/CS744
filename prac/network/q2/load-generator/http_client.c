#include "http_client.h"

void init_http_request(struct HTTP_Request *request, const char *url)
{
    strcpy(request->url, url);
    strcpy(request->HTTP_version, "1.0");
    strcpy(request->method, "GET");
}

void get_request_string(struct HTTP_Request *request, char *buffer)
{
    sprintf(buffer, "%s %s HTTP/%s\r\n\r\n", request->method, request->url, request->HTTP_version);
}