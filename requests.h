#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"

#ifndef _REQUESTS_
#define _REQUESTS_

// Each of the following functions will procees only one kind of request at a time
char *getReqBasic(const char *host, const char *url, const char *query_params,
                            char **cookies, int cookies_count) {
    // Request sent when demanding basic information
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count != 0) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; ++i) {
            sprintf(line, "%s; ", cookies[i]);
            strcat(message, line);
        }

        sprintf(line, "%s", cookies[cookies_count - 1]);
        strcat(message, line);
        compute_message(message, "");
    }
    compute_message(message, "");
    free(line);
    return message;
}

char *getReqAuthorized(const char *host, const char *url, const char *query_params,
                            const char *authorization, char **cookies, int cookies_count) {
    // Request sent when demanding protected information
    // Thus the authorization header is included
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count != 0) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; ++i) {
            sprintf(line, "%s; ", cookies[i]);
            strcat(message, line);
        }

        sprintf(line, "%s", cookies[cookies_count - 1]);
        strcat(message, line);

        compute_message(message, "");
    }

    sprintf(line, "Authorization: Bearer %s", authorization);
    compute_message(message, line);
    compute_message(message, "");
    free(line);
    return message;
}

char *postReqJson(const char *host, const char *url, const char* content_type,
					const char *body_data, char **cookies, int cookies_count) {
    // Basic post request primary used on login or registration
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message, line);


    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    if (cookies != NULL) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count; ++i) {
            sprintf(line, "%s; ", cookies[i]);
            strcat(message, line);
        }
        compute_message(message, "");
    }

    sprintf(line, "Content-Length: %ld", strlen(body_data));
    compute_message(message, line);
    compute_message(message, "");
    compute_message(message, body_data);

    free(line);
    return message;
}

char *postReqJsonAuthorized(const char *host, const char *url, const char* content_type,
					const char *authorization, const char *body_data, char **cookies, int cookies_count) {
    // Authorised post request primary used when adding a book in the lib
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    if (cookies != NULL) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count; ++i) {
            sprintf(line, "%s; ", cookies[i]);
            strcat(message, line);
        }
        compute_message(message, "");
    }

    sprintf(line, "Authorization: Bearer %s", authorization);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(body_data));
    compute_message(message, line);
    compute_message(message, "");
    compute_message(message, body_data);

    free(line);
    return message;
}


char *delReqJsonAuthorized(const char *host, const char *url, const char* query_params,
					const char *authorization, char **cookies, int cookies_count) {
    // Accessing the path to delete a book from lib
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count != 0) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; ++i) {
            sprintf(line, "%s; ", cookies[i]);
            strcat(message, line);
        }

        sprintf(line, "%s", cookies[cookies_count - 1]);
        strcat(message, line);

        compute_message(message, "");
    }

    sprintf(line, "Authorization: Bearer %s", authorization);
    compute_message(message, line);
    compute_message(message, "");

    free(line);
    return message;
}
#endif
