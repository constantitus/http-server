#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#include "http.h"
#include "helpers.h"

void *handler(void *arg) {
    int clientfd = *((int *)arg);
    char *buffer = (char *)malloc(1024 * sizeof(char));

    ssize_t bytes_received = recv(clientfd, buffer, 1024, 0);
    if (bytes_received > 0) {
        char *resp = (char *)malloc(1024 * 1024 * sizeof(char));
        size_t resp_len = 0;

        char header[45] = HTTP_HEADER_OK;
        string_cat(resp, header, 45 + 1);
        resp_len += 44;

        if (send(clientfd, resp, resp_len, 0) < 0) {
            perror("send error");
        }

        free(resp);
    }
    close(clientfd);
    free(arg);
    free(buffer);

    return NULL;
}

int main(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        AF_INET,
        htons(8080),
        0,
    };

    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
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

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handler, (void *)clientfd);
        pthread_detach(thread_id);
    }

    return 0;
}
