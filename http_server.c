#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "http_server.h"

http_request *http_request_new(int *fd, const http_server *server);

void http_free_request(http_request *r);

int _http_parse_first_line(http_request *r);

int _http_read_headers(http_request *r);


http_response_writer *http_response_writer_new();

void http_handle_writing(const http_request *r, http_response_writer *w);


http_server *http_server_new(http_serve_mux *handler, int port) {
    http_server *server = (http_server *)malloc(sizeof(http_server));
    server->handler = handler;
    server->addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    server->addr->sin_family = AF_INET;
    server->addr->sin_port = htons(port),
    server->addr->sin_addr.s_addr = 0;
    server->max_connections = 10;

    return server;
}

http_serve_mux *http_serve_mux_new() {
    http_serve_mux *mux = (http_serve_mux *)malloc(sizeof(http_serve_mux));
    // TODO: Assuming maximum of 16 handlers. Should realloc when exceeded.
    mux->funcs = (void (**)(http_request *, http_response_writer *))
        malloc(16 * sizeof(void *));
    mux->paths = (char **)malloc(16 * sizeof(char **));

    // Defaults path[0] to the root of the url ("/")
    mux->paths[0] = (char *)calloc(2, sizeof(char *));
    mux->paths[0][0] = '/';
    mux->funcs[0] = NULL; // Prevent errors
    mux->count = 0;

    return mux;
}

int http_handle(http_serve_mux *mux,
                char *pattern,
                void (*handler)(http_request *, http_response_writer *)) {
    if (!mux->funcs || !mux->paths)
        return -1;

    int idx = 0;
    if (strcmp(pattern, "/") == 0)
        mux->funcs[0] = handler;
    else
        idx = mux->count != 0 ? mux->count : 1;

    mux->paths[idx] = pattern;
    mux->funcs[idx] = handler;
    mux->count++;

    return 0;
}

static void *http_handler_mux(void *args) {
    http_request *r = (http_request *)args;

    if (_http_read_headers(r) < 0) {
        send(*r->fd, "HTTP/1.1 413 Entity Too Large\r\n\r\n", 34, 0);
        close(*r->fd);
        free(r);
        return NULL;
    }
    if (_http_parse_first_line(r) < 0) {
        free(r->header);
        close(*r->fd);
        free(r);
        return NULL;
    }

    http_serve_mux *mux = r->server->handler;
    pthread_mutex_lock(&mux->mu);

    void (*handler)(http_request *, http_response_writer *) = NULL;
    for (int i = 0; i < r->server->handler->count; i++) {
        if (strcmp(r->server->handler->paths[i], r->path) == 0) {
            handler = r->server->handler->funcs[i];
            break;
        }
    }
    if (!handler)
        handler = r->server->handler->funcs[0];
    pthread_mutex_unlock(&mux->mu);

    if (*handler) { // prevent errors if a handler for "/" was not defined
        http_response_writer *w = http_response_writer_new();
        handler(r, w);
        http_handle_writing(r, w);   
    }

    http_free_request(r);
    return NULL;
}

int http_listen_and_serve(const http_server *server) {
    if (server->handler->count == 0) {
        perror("No handlers defined");
        return -1;
    }

    // Catch SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    int sockfd = socket(server->addr->sin_family, SOCK_STREAM, 0);

    // Don't leave the socket open after closing the server.
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(sockfd,
             (struct sockaddr *)server->addr,
             sizeof(*server->addr)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(sockfd, server->max_connections) < 0) {
        perror("listen failed");
        return -1;
    }


    printf("Starting server on address %s:%d\n",
           inet_ntoa(server->addr->sin_addr), ntohs(server->addr->sin_port));

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *clientfd = (int *)malloc(sizeof(int));

        if ((*clientfd = accept(
            sockfd,
            (struct sockaddr *)&client_addr,
            &client_addr_len)) < 0) {
            perror("accept failed");
            continue;
        }

        http_request *r = http_request_new(clientfd, server);
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, http_handler_mux, r);
        pthread_detach(thread_id);
    }
    return 0;
}
