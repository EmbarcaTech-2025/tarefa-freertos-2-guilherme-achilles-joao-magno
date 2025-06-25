#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "pico/cyw43_arch.h"

/**
 * @brief Conecta-se a uma rede Wi-Fi usando o chip CYW43.
 *
 * Inicializa o hardware Wi-Fi e tenta conectar à rede especificada
 * pelo SSID e senha.
 *
 * @param ssid O nome da rede Wi-Fi (SSID).
 * @param password A senha da rede Wi-Fi.
 */
void connect_to_wifi(const char *ssid, const char *password);

#endif // WIFI_CONNECT_H
