#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "http_server.h"
#include "helpers.h"

#define MAX_HEADER_LENGTH 1024 * 1024


http_server *http_server_new() {
    http_server *server = (http_server *)malloc(sizeof(http_server));
    server->handlers = (http_handler_data *)malloc(sizeof(http_handler_data));
    server->handlers->func = malloc(64 * sizeof(void *(*)(http_request *)));
    server->handlers->path = (char **)malloc(64 * sizeof(char *));
    return server;
}

http_request *http_request_new(int fd, http_server *server) {
    http_request *r = (http_request *)malloc(sizeof(http_request));
    r->fd = fd;
    r->server = server;
    return r;
}

void http_free_request(http_request *r) {
    if (r->query) {
        free(r->query);
        r->query = NULL;
    }
    if (r->path) {
        free(r->path);
        r->path = NULL;
    }
    if (r->header) {
        free(r->header);
        r->header = NULL;
    }
    close(r->fd);
    free(r);
    r = NULL; // is this necessary ?
}

int http_handle(http_server *server,
                char *pattern,
                void (*handler)(http_request *)
                ) {
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

/*  Parses r->method, r->path and r->query from the first line of the header.
    The first line looks something like
    "GET /path?query_string=value HTTP/1.1\r\n".
    Returns -1 if error, (url not found, url doesn't end in ' ').
    Returns 0 if successful. */
static int http_parse_first_line(http_request *r) {
    if (!r->header) return -1;

    if (buffer_begins(r->header, "GET"))
        r->method = HTTP_GET;
    else if (buffer_begins(r->header, "POST"))
        r->method = HTTP_POST;

    int endl = string_find(r->header, "\r\n");
    int path_start = string_find_char(r->header, '/');
    // There should be a '/' on the first line.
    if (path_start > endl)
        return -1;

    char *tmp = r->header + path_start;
    endl -= path_start;

    int end = string_find_char(tmp, ' ');
    if (end <= 1) {
        r->path = (char *)malloc(sizeof(char) * 2);
        r->path[0] = '/';
        r->path[1] = '\0';
        return 0;
    }
    // There should also be a ' ' after the path or query string.
    if (end > endl)
        return -1;

    int qs_begin = string_find_char(tmp, '?');

    int path_end;
    if (qs_begin > 0 && qs_begin < end)
        path_end = qs_begin;
    else
        path_end = end;

    r->path = (char *)malloc(sizeof(char) * (path_end + 1));
    int i = 0;
    for (; i < path_end; i++)
        r->path[i] = tmp[i];

    if (qs_begin > 0 && qs_begin < end) {
        tmp += qs_begin + 1;
        r->query = (char *)malloc(sizeof(char) * (end - qs_begin + 1));
        i = 0;
        for (; i < (end - qs_begin - 1); i++)
            r->query[i] = tmp[i];
        r->query[i] = '\0';
    }
    return 0;
}

/*  Reads from the client fd until it reaches "\r\n\r\n".
    Result is stored in r->header
    Returns 0 if successful
    Returns -1 if the connection ended before reaching "\r\n\r\n" */
static int http_read_header(http_request *r) {
    r->header = (char *)malloc(sizeof(char) * MAX_HEADER_LENGTH);
    char *tmp = r->header;

    ssize_t received = 1;
    for (int i = 0;received > 0;i++, tmp++) {
        received = recv(r->fd, tmp, sizeof(char), 0);
        // TODO: rewrite this
        if (r->header[i]   == '\n' &&
            r->header[i-1] == '\r' &&
            r->header[i-2] == '\n' &&
            r->header[i-3] == '\r')
            return 0;
    }

    free(r->header);
    return -1;
}

static void *http_handler_mux(void *args) {
    http_request *r = (http_request *)args;

    if (http_read_header(r) < 0) {
        close(r->fd);
        free(r);
        return NULL;
    }

    if (http_parse_first_line(r) < 0) {
        free(r->header);
        close(r->fd);
        free(r);
        return NULL;
    }

    printf("\"%s\", \"%s\", %d, %d\n\n", r->path, r->query,
           string_len(r->path), string_len(r->query));

    for (int i = 0; i < r->server->handlers->count; i++) {
        if (strcmp(r->server->handlers->path[i], r->path) == 0) {
            r->server->handlers->func[i](r);
            http_free_request(r);
            return NULL;
        }
    }
    r->server->handlers->func[0](r);
    http_free_request(r);
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

        http_request *r = http_request_new(clientfd, server);
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, http_handler_mux, r);
        pthread_detach(thread_id);
    }
    return 0;
}


/* static void process_query_string(char *str) {
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
} */
