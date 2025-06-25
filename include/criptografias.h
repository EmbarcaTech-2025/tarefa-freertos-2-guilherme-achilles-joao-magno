#ifndef CRIPTOGRAFIAS_H
#define CRIPTOGRAFIAS_H

#include <stdint.h>
#include <stddef.h>
#include "mbedtls/aes.h" // Necessário se tipos da mbedtls fossem usados nas assinaturas ou para clareza da dependência

/**
 * @brief Criptografa/descriptografa dados usando uma cifra XOR simples.
 *
 * @param input Ponteiro para os dados de entrada.
 * @param output Ponteiro para o buffer onde os dados criptografados/descriptografados serão armazenados.
 * @param len Comprimento dos dados de entrada em bytes.
 * @param key A chave de 8 bits a ser usada para a operação XOR.
 */
void xor_encrypt_decrypt(const uint8_t *input, uint8_t *output, size_t len, uint8_t key);

/**
 * @brief Criptografa dados usando AES-128 no modo ECB.
 *
 * @param input Ponteiro para os dados de texto plano.
 * @param output Ponteiro para o buffer onde o texto cifrado será armazenado.
 * Este buffer deve ser grande o suficiente para conter o texto cifrado (mesmo tamanho da entrada, preenchido para blocos de 16 bytes).
 * @param len Comprimento dos dados de entrada em bytes. A função preencherá o último bloco com zeros se necessário.
 * @param key Ponteiro para a chave AES de 128 bits (16 bytes).
 */
void aes_encrypt(const uint8_t *input, uint8_t *output, size_t len, const uint8_t *key);

/**
 * @brief Descriptografa dados usando AES-128 no modo ECB.
 *
 * @param input Ponteiro para os dados de texto cifrado. O comprimento da entrada deve ser um múltiplo de 16 bytes.
 * @param output Ponteiro para o buffer onde o texto plano será armazenado.
 * Este buffer deve ser grande o suficiente para conter o texto plano (mesmo tamanho da entrada).
 * @param len Comprimento dos dados de entrada em bytes. Deve ser um múltiplo de 16.
 * @param key Ponteiro para a chave AES de 128 bits (16 bytes).
 */
void aes_decrypt(const uint8_t *input, uint8_t *output, size_t len, const uint8_t *key);

#endif // MY_CRYPTO_H