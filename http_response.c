#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "http_server.h"
#include "helpers.h"

http_response_writer *http_response_writer_new() {
    http_response_writer *w = (http_response_writer *)malloc(
        sizeof(http_response_writer));
    w->headers = (char **)malloc(32 * sizeof(void *));
    w->responses = (char **)malloc(32 * sizeof(void *));
    w->headers_count = 0;
    w->resp_count = 0;
    
    w->content_type = (char *)malloc(128 * sizeof(char));
    memcpy(w->content_type,
        "text/html; charset=utf-8",
        25);

    return w;
}

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

    // Set Content-Type
    http_set_header(w, "Content-Type", w->content_type);
    free(w->content_type);

    // Send the headers
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
}

// Do we even need a function for this ?
void http_set_status(http_response_writer *w, http_status status) {
    w->status = status;
}

int http_write(http_response_writer *w, const char *buf, size_t len) {
    if (!buf) return -1;
    if (!w->responses) {
        return -1; // Should not happen
    }

    w->responses[w->resp_count] = (char *)malloc(len * sizeof(char));
    string_copy(w->responses[w->resp_count], buf, len);
    w->resp_count++;

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
    memcpy(w->content_type, content_type, string_len(content_type) + 1);
}

int http_set_header(http_response_writer *w,
                    const char *name,
                    const char *value) {
    if (!w->headers)
        return -1; // Should not happen

    w->headers[w->headers_count] = (char *)malloc(256 * sizeof(char));
    int written = snprintf(w->headers[w->headers_count],
                        256,
                        "%s: %s\r\n",
                        name,
                        value);

    w->headers_count++;
    return 0;
}
