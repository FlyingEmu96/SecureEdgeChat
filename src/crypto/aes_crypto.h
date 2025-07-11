#ifndef AES_CRYPTO_H
#define AES_CRYPTO_H

#include <stddef.h>

int aes_encrypt(const unsigned char *plaintext, int plaintext_len,
                const unsigned char *key, const unsigned char *iv,
                unsigned char *ciphertext);

int aes_decrypt(const unsigned char *ciphertext, int ciphertext_len,
                const unsigned char *key, const unsigned char *iv,
                unsigned char *plaintext);

#endif // AES_CRYPTO_H

