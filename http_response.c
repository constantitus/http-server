#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "http_server.h"

http_response_writer *http_response_writer_new() {
    http_response_writer *w = (http_response_writer *)malloc(
        sizeof(http_response_writer));
    w->headers = (char **)malloc(32 * sizeof(void *));
    w->responses = (char **)malloc(32 * sizeof(void *));
    w->header_cnt = 0;
    w->resp_cnt = 0;
    w->status = NULL;
    
    w->content_type = (char *)malloc(128 * sizeof(char));
    memmove(w->content_type,
        "text/html; charset=utf-8",
        25);

    return w;
}

void http_handle_writing(const http_request *r, http_response_writer *w) {
    // TODO: Calling strlen too many times, just keep the lengths somewhere

    // Send the header
    if (w->status) {
        int len = strlen(w->status) + 12;
        char *tmp = (char *)malloc(len * sizeof(char));
        snprintf(tmp, len, "HTTP/1.1 %s\r\n", w->status);
        send(*r->fd, tmp, len, 0);
        free(w->status);
        free(tmp);
    } else {
        send(*r->fd, "HTTP/1.1 200 OK\r\n", 17, 0);
    }

    // Set Content-Type
    http_set_header(w, "Content-Type", w->content_type);
    free(w->content_type);

    // Send the headers
    for (int i = 0; i < w->header_cnt; i++) {
        if (!w->headers[i]) continue;
        send(*r->fd, w->headers[i], strlen(w->headers[i]), 0);
        free(w->headers[i]);
        w->headers[i] = NULL;
    }

    free(w->headers);
    w->headers = NULL;

    send(*r->fd, "\r\n", 2, 0);

    // Send the response
    if (w->resp_cnt > 0) {
        for (int i = 0; i < w->resp_cnt; i++) {
            if (!w->responses[i]) continue;
            send(*r->fd, w->responses[i], strlen(w->responses[i]), 0);
            free(w->responses[i]);
            w->responses[i] = NULL;
        }
    }
    free(w->responses);
    w->responses = NULL;
    free(w);
}

int http_set_status(http_response_writer *w, char *status) {
    if (!*status)
        return -1;
    if (w->status) {
        puts("http_set_status error: You are trying to set the status twice");
        return -2;
    }

    int len = strlen(status);
    w->status = (char *)malloc(len * sizeof(char));
    memmove(w->status, status, len);
    return 0;
}

int http_write(http_response_writer *w, const char *buf, size_t len) {
    if (!*buf) return -1;

    w->responses[w->resp_cnt] = (char *)malloc(len * sizeof(char));
    strncpy(w->responses[w->resp_cnt], buf, len); // Yes
    w->resp_cnt++;

    return 0;
}

int http_set_cookie(http_response_writer *w,
                    const char *name,
                    const char *value,
                    ...) {
    // TODO: variadic args for more cookie values.
    char *cookie = (char *)malloc(128 * sizeof(char));
    char *tmp = cookie + snprintf(cookie, 128, "%s=%s", name, value);

    int res = http_set_header(w, "Set-Cookie", cookie);
    free(cookie);
    return res;
}

void http_set_content_type(http_response_writer *w, const char* content_type) {
    memmove(w->content_type, content_type, strlen(content_type) + 1);
}

int http_set_header(http_response_writer *w,
                    const char *name,
                    const char *value) {
    if (!w->headers)
        return -1; // Should not happen

    w->headers[w->header_cnt] = (char *)malloc(256 * sizeof(char));
    int written = snprintf(w->headers[w->header_cnt],
                        256,
                        "%s: %s\r\n",
                        name,
                        value);

    w->header_cnt++;
    return 0;
}
