#include <stdint.h>
#include "neopixel_pio.h"

void matriz_dinamic(float temperatura)
{

    npClear();

    // Faixas de temperatura e LEDs
    struct {
        float limite_superior;
        uint8_t start_led;
        uint8_t end_led;
        uint8_t r, g, b;
    } faixas[] = {
        {10.0f, 0, 4, 0, 0, 128},
        {30.0f, 5, 9, 0, 0, 160},
        {40.0f, 10, 14, 0, 180, 0},
        {50.0f, 15, 19, 255, 165, 0},
        {1000.0f, 20, 24, 255, 0, 0} // Limite superior arbitrário
    };

    for (int i = 0; i < 5; i++) {
        if (temperatura > faixas[i].limite_superior)
            continue;

        // Acende todos os LEDs até a faixa atual
        for (int j = 0; j <= i; j++) {
            for (uint8_t k = faixas[j].start_led; k <= faixas[j].end_led; k++) {
                npSetLED(k, faixas[j].r, faixas[j].g, faixas[j].b);
            }
        }

        break; // Sai após pintar a faixa correta
    }

    npWrite();
}
