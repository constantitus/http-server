#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http.h"

void *index_handler(void *arg) {
    int clientfd = (uintptr_t)arg;
    char *buffer = (char *)malloc(1024 * sizeof(char));

    ssize_t bytes_received = recv(clientfd, buffer, 1024, 0);
    if (bytes_received > 0) {
        printf("%s\n", buffer);

        /* char *resp = (char *)malloc(1024 * 1024 * sizeof(char));
        size_t resp_len = 0;
        if (send(clientfd, resp, resp_len, 0) < 0) {
            perror("send error");
        }
        free(resp); */
    }
    close(clientfd);
    free(buffer);

    return NULL;
}

int main(void) {
    http_server *server = http_server_new();

    http_handle(server, "/", index_handler);

    http_listen_and_serve(server, 8080);

    return 0;
}
