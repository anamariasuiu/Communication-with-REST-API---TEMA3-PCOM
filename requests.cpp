#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(const char *host, const char *url, const char *query_params,
                            char **cookies, int cookies_count, const char* token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        for(int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s",cookies[i]);
            compute_message(message, line);
        }
    }
    //adaugam tokenul
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s",token);
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(const char *host, const char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, const char *token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
    memset(body_data_buffer, 0, LINELEN);
    int body_data_size = 0;
    if (body_data != NULL && body_data_fields_count) {
        for (int i = 0; i < body_data_fields_count - 1; i++) {
            strcat(body_data_buffer, body_data[i]);
            strcat(body_data_buffer, "&");
            body_data_size += strlen(body_data[i]);
            body_data_size++; //for '&
        }
    }

    strcat(body_data_buffer, body_data[body_data_fields_count - 1]);
    body_data_size += strlen(body_data[body_data_fields_count - 1]);

    sprintf(line, "Content-Type: %s\r\nContent-Length: %d", content_type, body_data_size);
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {//verificare existenta cookie
        for(int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s",cookies[i]);
            compute_message(message, line);
        }
    }

    // adaugam tokenul
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s",token);
        compute_message(message, line);
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add data
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}
char *compute_delete_request(const char *host, const char *url,const char *query_params,
                            char **cookies, int cookies_count,const char *token)
{
    char *message = (char*)calloc(BUFLEN, sizeof(char));
    char *line = (char*)calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);

    compute_message(message, line);


    //adaugam cookie
    if (cookies != NULL) {//verificare existenta cookie
        for(int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s",cookies[i]);
            compute_message(message, line);
        }
    }
    //adaugam token
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s",token);
        compute_message(message, line);
    }

    compute_message(message, "");
    free(line);
    return message;
}