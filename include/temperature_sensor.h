#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <stdint.h>

// Variável global para armazenar o resultado da conversão ADC
extern volatile uint16_t adc_result;

// Inicializa o ADC e o sensor de temperatura interno
void init_temp(void);

// Configura a FIFO e as interrupções do ADC
void init_fifo_temp(void);

// Dispara manualmente a conversão ADC (modo single conversion)
void trigger_adc(void);

// Função para converter o valor do ADC para tensão
float get_voltage(void);

// Handler da interrupção ADC
void adc_irq(void);

#endif // TEMPERATURE_SENSOR_H
