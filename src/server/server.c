#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"
#include "utils.h"
#include <pthread.h>

#define PORT 9090
#define BACKLOG 5

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);  // we malloc'd it earlier

    char buffer[1024] = {0};
    ssize_t len;

    while ((len = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[len] = '\0';
        printf("Client [%d]: %s\n", client_fd, buffer);
        send(client_fd, buffer, len, 0);  // Echo back for now
    }

    close(client_fd);
    printf("Client [%d] disconnected.\n", client_fd);
    return NULL;
}

int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int *client_fd = malloc(sizeof(int));
        if (!client_fd) {
            perror("malloc failed");
            continue;
        }

        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd == -1) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create failed");
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(tid); // Automatically reclaim thread resources
        }
    }

    close(server_fd);  // unreachable, but good form
    return EXIT_SUCCESS;
}

