#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "http_server.h"
#include "helpers.h"

http_response_writer *http_response_writer_new() {
    http_response_writer *w = malloc(sizeof(http_response_writer));
    w->headers = (char **)malloc(sizeof(void *) * 32);
    w->responses = (char **)malloc(sizeof(void *) * 256);
    w->headers_count = 0;
    w->resp_count = 0;

    return w;
}

// TODO: send everything in a single call to send()

void http_handle_writing(const http_request *r, http_response_writer *w) {
    // Send the header
    // TODO: Either add more status codes or just let the user set the status.
    switch (w->status) {
        case HTTP_NOT_FOUND:
            send(*r->fd, "HTTP/1.1 404 Not Found\r\n", 24, 0);
            break;
        default: // case HTTP_OK:
            send(*r->fd, "HTTP/1.1 200 OK\r\n", 17, 0);
            break;
    }
    // TODO: Let the user set the Content-Type
    send(*r->fd, "Content-Type: text/html; charset=utf-8\r\n", 40, 0);

    // Send the cookies
    for (int i = 0; i < w->headers_count; i++) {
        if (!w->headers[i]) continue;
        send(*r->fd, w->headers[i], string_len(w->headers[i]), 0);
        free(w->headers[i]);
        w->headers[i] = NULL;
    }

    free(w->headers);
    w->headers = NULL;

    send(*r->fd, "\r\n", 2, 0);

    // Send the response
    if (w->resp_count > 0) {
        for (int i = 0; i < w->resp_count; i++) {
            if (!w->responses[i]) continue;
            send(*r->fd, w->responses[i], string_len(w->responses[i]), 0);
            free(w->responses[i]);
            w->responses[i] = NULL;
        }
    }
    free(w->responses);
    w->responses = NULL;
    free(w);
    w = NULL;
}


int http_set_status(http_response_writer *w, http_status status) {
    w->status = status;
    return 0;
}

int http_write(http_response_writer *w, const char *buf, size_t len) {
    if (!buf) return -1;
    if (!w->responses) {
        return -1;
    }

    w->responses[w->resp_count] = malloc(len * sizeof(char));
    string_copy(w->responses[w->resp_count], buf, len);
    w->resp_count++;

    return 0;
}

int http_set_cookie(http_response_writer *w,
                    const char *name,
                    const char *value,
                    ...) {

    char *cookie = (char *)malloc(128 * sizeof(char));
    char *tmp = cookie + snprintf(cookie, 128, "%s=%s", name, value);

    http_set_header(w, "Set-Cookie", cookie);
    free(cookie);
    
    return 0;
}


int http_set_header(http_response_writer *w,
                    const char *name,
                    const char *value) {
    w->headers[w->headers_count] = (char *)malloc(256 * sizeof(char));
    int written = snprintf(w->headers[w->headers_count],
                        256,
                        "%s: %s\r\n",
                        name,
                        value);
    if (written < 256) {
        w->headers[w->headers_count] = realloc(w->headers[w->headers_count],
                                              (256 + written) * sizeof(char));
        snprintf(w->headers[w->headers_count],
                 256,
                 "%s: %s\r\n",
                 name,
                 value);
    }

    w->headers_count++;
    return 0;
}
