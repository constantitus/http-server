#ifndef HTTP_H
#define HTTP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "helpers.h"

typedef struct {
    void *(**func)(void *);
    char **path;
    int count;
} http_handler_data;

typedef struct {
    http_handler_data *handlers;
} http_server;


typedef struct {
    int fd;
    http_server *server;
    char *method;
    char *path;
    char *query;
} http_request;

/*  http_server initializer.
    Result must be freed */
http_server *http_server_new();

/*  http_request initializer
    Result must be freed */
http_request *http_request_new(int fd, http_server *server);

void http_request_free(http_request *r);

/*  Set a handler function for a specific path */
int http_handle(http_server *server, char *pattern, void *(*handler)(void *));

/*  Start the server */
int http_listen_and_serve(http_server *server, int port);

/*  Reads the first line from the HTTP request and parses it into 
    r->method, r->path and r->query.
    Returns -1 if error. */
int http_read_first(http_request *request);



http_server *http_server_new() {
    http_server *server = (http_server *)malloc(sizeof(http_server));
    server->handlers = (http_handler_data *)malloc(sizeof(http_handler_data));
    server->handlers->func = malloc(64 * sizeof(void *(*)(void *)));
    server->handlers->path = malloc(64 * sizeof(char *));
    return server;
}

http_request *http_request_new(int fd, http_server *server) {
    http_request *r = malloc(sizeof(http_request));
    r->fd = fd;
    r->server = server;
    return r;
}

void http_request_free(http_request *r) {
    if (r->path) free(r->path);
    if (r->query) free(r->query);
    if (r->method) free(r->method);
}

int http_handle(http_server *server, char *pattern, void *(*handler)(void *)) {
    int count = 0;
    if (strcmp(pattern, "/") != 0) {
        count = server->handlers->count;
        server->handlers->count++;
    }
    server->handlers->func[count] = handler;
    server->handlers->path[count] = malloc(sizeof(char) * strlen(pattern));
    server->handlers->path[count] = pattern;
    server->handlers->count++;
    return 0;
}

int http_read_first(http_request *r) {
    char *buffer = (char *)malloc(sizeof(char) * 2048);
    char *original = buffer;

    ssize_t received;
    int i = 0;
    do {
        received = recv(r->fd, buffer, sizeof(char), 0);
        if (original[i] == '\n')
            break;
        buffer++;
        i++;
    } while (received > 0);
    buffer = original;

    int method_len = string_find_char(buffer, ' ');
    r->method = malloc(sizeof(char) * 6);
    for (i = 0; i < method_len; i++) {
        r->method[i] = buffer[i];
    }
    r->method[i] = '\0'; // what the fuck
    buffer += method_len + 1;

    int end = string_find_char(buffer, ' ');
    int end_path = end;

    int qs_begin = string_find_char(buffer, '?');
    if (qs_begin > 0)
        end_path = qs_begin;


    r->path = malloc(sizeof(char) * 2048);
    for (i = 0; i < end; i++)
        r->path[i] = buffer[i];


    if (qs_begin > 0) {
        buffer += qs_begin + 1;
        r->query = malloc(sizeof(char) * 2048);
        for (i = 0; i < (end - qs_begin - 1); i++)
            r->query[i] = buffer[i];
    }

    free(original);

    // puts(r->method);
    // puts(r->path);
    // puts(r->query);
    return 0;
}

static void *_http_handler_mux(void *args) {
    // TODO: mux
    http_request *r = (http_request *)args;

    if (http_read_first(r) < 0) {
        perror("http_read_first");
        http_request_free(r);
        return NULL;
    }

    for (int i = 0; i < r->server->handlers->count; i++) {
        if (strcmp(r->server->handlers->path[i], r->path) == 0) {
            r->server->handlers->func[i]((void *)(uintptr_t)r->fd);
            http_request_free(r);
            return NULL;
        }
    }
    r->server->handlers->func[0]((void *)(uintptr_t)r->fd);
    http_request_free(r);
    return NULL;
}

int http_listen_and_serve(http_server *server, int port) {
    if (server->handlers->count == 0) {
        perror("no handlers");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        AF_INET,
        htons(port),
        0,
    };

    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen failed");
        return -1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int clientfd;

        if ((clientfd = accept(
            sockfd,
            (struct sockaddr *)&client_addr,
            &client_addr_len)) < 0) {
            perror("accept failed");
            continue;
        }


        http_request *data = http_request_new(clientfd, server);
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, _http_handler_mux, (void *)data);
        pthread_detach(thread_id);
    }
    return 0;
}

static void process_query_string(char *str) {
    // replace + with space
    char *tmp = str;
    int n = string_find_char(tmp, '+');
    while (n != -1) {
        tmp += n;
        *tmp = ' ';
        n = string_find_char(tmp, '+');
    }

    int i = 0;
    tmp = str;
    while (str[i]) {
        while (str[i] && str[i] != '%') {
            i += 1;
        }
        if (str[i] == '%') {
            tmp = &(str[i]);
            int hex = 0;
            i += 1;
            int j = 1;
            while (j >= 0) {
                if (str[i] >= 'A' && str[i] <= 'F') {
                    hex += (str[i] - 'A') + 10;
                }
                if (str[i] >= '0' && str[i] <= '9') {
                    hex += (str[i] - '0');
                }
                if (j) {
                    hex *= 16;
                }
                j -= 1;
                i += 1;
            }
            tmp[0] = hex;
            tmp += 1;
            string_shift(tmp);
            string_shift(tmp);
            i -= 2;
        }
    }
}

#endif
