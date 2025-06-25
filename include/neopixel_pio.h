#ifndef NEOPIXEL_PIO_H
#define NEOPIXEL_PIO_H

#include <stdint.h>
#include "pico/types.h" // Adicione isso para reconhecer o tipo uint

void npInit(uint pin, uint amount);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite(); // Corrigido: sua implementação usa npWrite, não npShow

#endif // NEOPIXEL_PIO_H
