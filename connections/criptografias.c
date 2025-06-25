#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "mbedtls/aes.h"

// Função XOR (mantida igual)
void xor_encrypt_decrypt(const uint8_t *input, uint8_t *output, size_t len, uint8_t key) {
    for (size_t i = 0; i < len; ++i) {
        output[i] = input[i] ^ key;
    }
}

// Adiciona padding PKCS#7
static void add_padding(uint8_t *block, size_t data_len, size_t block_size) {
    uint8_t pad_len = block_size - (data_len % block_size);
    for (size_t i = data_len; i < block_size; ++i) {
        block[i] = pad_len;
    }
}

// Remove padding PKCS#7
static size_t remove_padding(const uint8_t *block, size_t block_size) {
    uint8_t pad_len = block[block_size - 1];
    return (pad_len <= block_size) ? (block_size - pad_len) : block_size;
}

// Criptografa com AES-128-ECB (com padding PKCS#7)
void aes_encrypt(const uint8_t *input, uint8_t *output, size_t len, const uint8_t *key) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128);

    const size_t block_size = 16;
    size_t padded_len = len + (block_size - (len % block_size));
    uint8_t padded_input[padded_len];

    // Copia dados e adiciona padding
    memcpy(padded_input, input, len);
    add_padding(padded_input, len, block_size);

    // Criptografa cada bloco
    for (size_t i = 0; i < padded_len; i += block_size) {
        mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, padded_input + i, output + i);
    }

    mbedtls_aes_free(&aes);
}

// Descriptografa com AES-128-ECB (e remove padding PKCS#7)
void aes_decrypt(const uint8_t *input, uint8_t *output, size_t len, const uint8_t *key) {
    if (len % 16 != 0) return; // Tamanho inválido

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, key, 128);

    // Descriptografa cada bloco
    for (size_t i = 0; i < len; i += 16) {
        mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input + i, output + i);
    }

    // Remove padding do último bloco
    size_t data_len = remove_padding(output + len - 16, 16);
    memset(output + len - 16 + data_len, 0, 16 - data_len); // Limpa bytes extras

    mbedtls_aes_free(&aes);
}