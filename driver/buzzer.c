
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdlib.h>
#include <stdio.h>
#include "hardware/clocks.h"


// Configuração do pino do buzzer
#define BUZZER_PIN 21

void pwm_init_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_wrap(slice, 4096);

    // Vamos gerar PWM em ≈ 20 kHz
    float clkdiv = (float)clock_get_hz(clk_sys) / (30000 *4096);
    pwm_set_clkdiv(slice,clkdiv);
    pwm_set_enabled(slice, true); // Habilita o PWM no slice
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void beep_buzzer (void) {
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_gpio_level(BUZZER_PIN, 2048); // Define o nível do PWM para 50% (2048/4096)
    sleep_ms(100); // Duração do beep
    pwm_set_gpio_level(BUZZER_PIN, 0); // Desliga o buzzer
}


