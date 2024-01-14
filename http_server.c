#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "http_server.h"
#include "helpers.h"

http_request *http_request_new(int *fd, const http_server *server);

void http_free_request(http_request *r);

int http_parse_first_line(http_request *r);

int http_read_header(http_request *r);


http_response_writer *http_response_writer_new();

void http_handle_writing(const http_request *r, http_response_writer *w);



http_server *http_server_new() {
    http_server *server = (http_server *)malloc(sizeof(http_server));
    server->handlers = (http_handlers **)malloc(64 * sizeof(void *));
    server->addr = "";
    server->port = 8080;
    
    return server;
}

int http_handle(http_server *server,
                char *pattern,
                void (*handler)(http_request *, http_response_writer *)
                ) {
    if (!server->handlers)
        return -1; // Should never happen

    int count = 0;
    if (strcmp(pattern, "/") != 0) {
        count = server->handlers_count;
        server->handlers_count++;
    }
    server->handlers[count] = malloc(sizeof(http_handlers));
    server->handlers[count]->func = handler;
    server->handlers[count]->path = malloc(sizeof(char) * strlen(pattern));
    server->handlers[count]->path = pattern;
    server->handlers_count++;
    return 0;
}

static void *http_handler_mux(void *args) {
    http_request *r = (http_request *)args;

    if (http_read_header(r) < 0) {
        send(*r->fd, "HTTP/1.1 413 Entity Too Large\r\n\r\n", 34, 0);
        close(*r->fd);
        free(r);
        return NULL;
    }

    if (http_parse_first_line(r) < 0) {
        free(r->header);
        close(*r->fd);
        free(r);
        return NULL;
    }

    http_response_writer *w = http_response_writer_new();

    for (int i = 0; i < r->server->handlers_count; i++) {
        if (strcmp(r->server->handlers[i]->path, r->path) == 0) {
            r->server->handlers[i]->func(r, w);
            http_handle_writing(r, w);   
            http_free_request(r);
            return NULL;
        }
    }
    // Defaults to "/"
    r->server->handlers[0]->func(r, w);
    http_handle_writing(r, w);   
    http_free_request(r);
    return NULL;
}

int http_listen_and_serve(const http_server *server) {
    if (server->handlers_count == 0) {
        perror("No handlers defined");
        return -1;
    }

    // Catch SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        AF_INET,
        htons(server->port),
        0,
    };
    if (string_len(server->addr))
        addr.sin_addr.s_addr = inet_addr(server->addr);

    // Don't leave the socket open after closing the server.
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));


    printf("Starting server on address %s:%d\n",
           inet_ntoa(addr.sin_addr), server->port);

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
        int *clientfd = malloc(sizeof(int));

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
