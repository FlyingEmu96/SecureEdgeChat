#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "client.h"
#include "utils.h"
#include "aes_crypto.h"

#define PORT 9090

unsigned char key[32] = "01234567890123456789012345678901";
unsigned char iv[16]  = "1234567890123456";

int main(void) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect failed");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("Connected to server. Type your message (/exit to quit):\n");

    char input[1024];
    unsigned char encrypted[1024];
    unsigned char recv_buf[1024];
    unsigned char decrypted[1024];

    while (1) {
        printf("You: ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("Input stream closed.\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "/exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        int enc_len = aes_encrypt((unsigned char*)input, strlen(input), key, iv, encrypted);
        if (enc_len > 0) {
            send(sock_fd, encrypted, enc_len, 0);
        }

        ssize_t recv_len = recv(sock_fd, recv_buf, sizeof(recv_buf), 0);
        if (recv_len <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        int dec_len = aes_decrypt(recv_buf, recv_len, key, iv, decrypted);
        if (dec_len > 0) {
            decrypted[dec_len] = '\0';
            printf("%s\n", decrypted);
        } else {
            fprintf(stderr, "Decryption failed.\n");
        }
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}

