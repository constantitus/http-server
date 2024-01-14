#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "http_server.h"

void index_handler(http_request *r, http_response_writer *w) {

    char *buffer = (char *)calloc(1024, sizeof(char));
    ssize_t bytes_received = recv(*r->fd, buffer, 1024, 0);
    if (bytes_received > 0) {
        printf("%s\n", buffer);
    }
    free(buffer);

    http_write(w, 
               "<!Doctype html>"
               "<html>"
               "<head><title>Test page</title></head>"
               "<body><h1>Test page</h1>",
               83);
    http_set_cookie(w, "test", "test-value");
}

#include<unistd.h>

typedef struct {
    char *mikasa;
    int which_mikasa;
} nigga_chin;

int main(void) {
    http_server *server = http_server_new();

    // Optional parameters
    server->addr = "127.0.0.1"; // Defaults to 0.0.0.0
    server->port = 8080;        // Defaults to 8080

    http_handle(server, "/", index_handler);

    http_listen_and_serve(server);

    return 0;
}
