#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "http_server.h"
#include "helpers.h"

void index_handler(http_request *r, http_response_writer *w) {
    http_write(w, "<!Doctype html>"
               "<html>"
               "<head><title>Test page</title></head>"
               "<body><h1>Title</h1>"
               "<form action=\"/recv\" method=\"post\" "
               "enctype=\"multipart/form-data\">"
               "<input type=\"file\" name=\"file\" multiple=\"multiple\">"
               "<input type=\"submit\" value=\"submit\">"
               "</form>"
               "</body>"
               "</html>",
               252);
    http_set_cookie(w, "test", "test-value");
}

void recv_handler(http_request *r, http_response_writer *w) {
    http_set_content_type(w, "text/plain");

    char *boundary = http_multipart_get_boundary(r);
    if (!boundary) {
        http_write(w, "Failed", 7);
        return;
    }
    // printf("\"%s\"\n", boundary);


    char *buffer = (char *)malloc(1024 * sizeof(char));
    ssize_t bytes_received = recv(*r->fd, buffer, 1024, 0);
    // if (bytes_received > 0) printf("%s\n", buffer);
    if (bytes_received < 1) {
        free(buffer);
        free(boundary);
        return;
    }
    // printf("[%s]\n", boundary);

    // printf("Buffer:[%s]\n", buffer);
    int *count = (int *)malloc(sizeof(int));
    char **fields = string_split(buffer, boundary, count);

    // Do something with the fields
    for (int i = 0; i < *count; i++) {
        printf("%d:[%s]\n", i, fields[i]);
        free(fields[i]);
    }
    http_write(w, "Received", 9);


    free(count);
    
    free(buffer);
    free(boundary);
    if (fields)
        free(fields);
    return;
}

int main(void) {
    http_serve_mux *mux = http_serve_mux_new();

    http_handle(mux, "/recv", recv_handler);
    http_handle(mux, "/", index_handler);

    http_server *server = http_server_new(mux, 8080);
    // Optional parameters
    inet_aton("127.0.0.1", &server->addr->sin_addr); // Defaults to 0.0.0.0
    server->max_connections = 20; // Defaults to 10

    http_listen_and_serve(server);

    return 0;
}
