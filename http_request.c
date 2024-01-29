#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "helpers.h"
#include "http_server.h"

#define MAX_HEADER_LENGTH 1024 * 8

http_request *http_request_new(int *fd, const http_server *server) {
    http_request *r = (http_request *)malloc(sizeof(http_request));
    r->query = NULL;
    r->path = NULL;
    r->header = NULL;
    r->fd = fd;
    r->server = server;
    return r;
}

void http_free_request(http_request *r) {
    if (r->query)
        free(r->query);
    if (r->path)
        free(r->path);
    if (r->header)
        free(r->header);
    close(*r->fd);
    free(r->fd);
    free(r);
}

char *http_multipart_get_boundary(http_request *r) {
    // ?????
    char *content_type = http_get_header(r, "Content-Type");
    if (!content_type)
        return NULL;

    int start = str_findstr(content_type, "multipart/form-data; boundary=");
    if (start < 0) {
        free(content_type);
        return NULL;
    }

    char *tmp = content_type + start + 30;

    int end = strlen(tmp);
    char *boundary = (char *)malloc((end + 1) * sizeof(char));
    memmove(boundary, tmp, end);
    boundary[end] = '\0';
    
    free(content_type);
    return boundary;
}

char *http_get_header(http_request *r, const char *name) {
    if (!*name)
        return NULL;

    char *tmp = r->header;

    char *fullname = (char *)malloc(64 * sizeof(char));
    memmove(fullname, "\r\n", 2);
    memmove(fullname + 2, name, strlen(name) + 1);
    
    int start = str_findstr(tmp, fullname);

    tmp += start + strlen(fullname);
    free(fullname);

    if (start < 0)
        return NULL;

    if (tmp[0] != ':' && tmp[1] != ' ')
        return NULL;

    tmp += 2;

    int end = str_findstr(tmp, "\r\n");

    char *header = (char *)malloc((end + 1) * sizeof(char));
    int i = 0;
    for (; i < end; i++)
        header[i] = tmp[i];
    header[i] = '\0';

    return header;
}

/*  Parses r->method, r->path and r->query from the first line of the header.
    The first line looks something like
    "GET /path?query_string=value HTTP/1.1\r\n".
    Returns -1 if error, (url not found, url doesn't end in ' ').
    Returns 0 if successful. */
int _http_parse_first_line(http_request *r) {
    if (str_starts(r->header, "GET"))
        r->method = HTTP_GET;
    else if (str_starts(r->header, "POST"))
        r->method = HTTP_POST;

    int endl = str_findstr(r->header, "\r\n");
    const int path_start = str_findchar(r->header, '/');
    // There should be a '/' on the first line.
    if (path_start > endl)
        return -1;

    char *tmp = r->header + path_start;
    endl -= path_start;

    const int end = str_findchar(tmp, ' ');
    if (end <= 1) {
        r->path = (char *)malloc(2 * sizeof(char));
        r->path[0] = '/';
        r->path[1] = '\0';
        return 0;
    }
    // There should also be a ' ' after the path or query string.
    if (end > endl)
        return -1;

    const int qs_begin = str_findchar(tmp, '?');

    int path_end;
    if (qs_begin > 0 && qs_begin < end)
        path_end = qs_begin;
    else
        path_end = end;

    r->path = (char *)malloc((path_end + 1) * sizeof(char));
    int i = 0;
    for (; i < path_end; i++)
        r->path[i] = tmp[i];
    r->path[i] = '\0';

    if (qs_begin > 0 && qs_begin < end) {
        tmp += qs_begin + 1;
        r->query = (char *)malloc((end - qs_begin + 1) * sizeof(char));
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
    Returns -1 if the connection ended before reaching "\r\n\r\n" or if the
    header was too large to fit the buffer */
int _http_read_headers(http_request *r) {
    r->header = (char *)malloc(MAX_HEADER_LENGTH * sizeof(char));
    char *tmp = r->header;

    ssize_t received = 1;
    for (int i = 0;received > 0 && i < MAX_HEADER_LENGTH ;i++, tmp++) {
        received = recv(*r->fd, tmp, sizeof(char), 0);
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

