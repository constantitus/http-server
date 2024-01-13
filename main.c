#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "http_server.h"

void index_handler(http_request *r) {
    /* char *buffer = (char *)malloc(1024 * sizeof(char));
    ssize_t bytes_received = recv(r->fd, buffer, 1024, 0);
    if (bytes_received > 0) {
        printf("%s\n", buffer);

    }
    free(buffer); */

    /* char *resp = (char *)malloc(1024 * 1024 * sizeof(char));
    size_t resp_len = 0;
    if (send(r->fd, resp, resp_len, 0) < 0) {
        perror("send error");
    }
    free(resp); */
}

int main(void) {
    http_server *server = http_server_new();

    http_handle(server, "/", index_handler);

    http_listen_and_serve(server, 8080);

    return 0;
}
