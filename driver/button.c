#include "pico/stdlib.h"
#include <stdio.h>
#include <stdbool.h>
#include "write_display.h"

#define BUTTON_A 5
#define BUTTON_B 6
#define DELAY_BUTTONS_US 150000

bool volatile button_a_nivel = false;
bool volatile button_b_nivel = false;

void init_buttons(void){
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
}

void button_isr(uint gpio, uint32_t events)
{
    uint32_t tempo_instantaneo = time_us_32();
    static uint32_t tempo_anterior = 0;

    if (tempo_instantaneo - tempo_anterior > DELAY_BUTTONS_US)
    {
        if (gpio == BUTTON_A)
        {
            button_a_nivel = 1;
        }
        if (gpio == BUTTON_B)
        {
            button_b_nivel = 1;
        }
    }
    tempo_anterior = tempo_instantaneo;
}

void init_button_interrupts(void)
{
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_isr);
}

void deinit_button_interrupts(void) 
{
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, false);
}

