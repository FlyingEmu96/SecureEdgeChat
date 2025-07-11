#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include "aes_crypto.h"

#define PORT 9090
#define BACKLOG 5
#define MAX_CLIENTS 100

unsigned char key[32] = "01234567890123456789012345678901";
unsigned char iv[16]  = "1234567890123456";

int client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;

void broadcast_to_all(const unsigned char *msg, int len, int sender_fd) {
    pthread_mutex_lock(&client_lock);
    for (int i = 0; i < client_count; i++) {
        int fd = client_sockets[i];
        if (fd != sender_fd) {
            send(fd, msg, len, 0);
        }
    }
    pthread_mutex_unlock(&client_lock);
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    uuid_t client_uuid;
    char uuid_str[37];
    uuid_generate(client_uuid);
    uuid_unparse(client_uuid, uuid_str);

    // Register this client
    pthread_mutex_lock(&client_lock);
    if (client_count < MAX_CLIENTS) {
        client_sockets[client_count++] = client_fd;
    }
    pthread_mutex_unlock(&client_lock);

    printf("Client [%s] connected.\n", uuid_str);

    unsigned char buffer[1024];
    ssize_t len;

    while ((len = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        unsigned char decrypted[1024] = {0};
        int dec_len = aes_decrypt(buffer, len, key, iv, decrypted);
        if (dec_len <= 0) break;

        decrypted[dec_len] = '\0';

        char response[1024] = {0};

        if (strncmp((char*)decrypted, "/me ", 4) == 0) {
            snprintf(response, sizeof(response), "[*%s*] %s", uuid_str, decrypted + 4);
        } else {
            snprintf(response, sizeof(response), "[%s]: ", uuid_str);
            size_t prefix_len = strlen(response);
            size_t space_left = sizeof(response) - prefix_len - 1;
            strncat(response, (char*)decrypted, space_left);
        }

        printf("%s\n", response);

        unsigned char encrypted[1024] = {0};
        int enc_len = aes_encrypt((unsigned char*)response, strlen(response), key, iv, encrypted);
        if (enc_len > 0) {
            send(client_fd, encrypted, enc_len, 0);              // echo back
            broadcast_to_all(encrypted, enc_len, client_fd);     // send to others
        }
    }

    // Remove client
    pthread_mutex_lock(&client_lock);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] == client_fd) {
            client_sockets[i] = client_sockets[--client_count];
            break;
        }
    }
    pthread_mutex_unlock(&client_lock);

    close(client_fd);
    printf("Client [%s] disconnected.\n", uuid_str);
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

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
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
        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd == -1) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return EXIT_SUCCESS;
}
