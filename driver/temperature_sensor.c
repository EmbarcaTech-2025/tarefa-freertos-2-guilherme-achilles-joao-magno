#include <hardware/adc.h>
#include <hardware/irq.h>

volatile uint16_t adc_result = 0;

void init_temp(void)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
}

void adc_irq(void)
{
    adc_result = adc_fifo_get(); // Ler o valor armazenado na FIFO
}

float get_voltage (void)
{
    const float conversion_factor = 3.3f / (1 << 12); // Fator de conversão para tensão
    float voltage = adc_result * conversion_factor; // Converte o valor lido para tensão
    return voltage;
}

void init_fifo_temp(void)
{
    adc_select_input(4);

    adc_fifo_setup(
        true, // Ativa a fifo
        true, // Ativa o data request (toda vez que a fifo tiver dados ela vai ser disparada)
        1,
        false,
        false
    );

    adc_fifo_drain (); //limpa a fifo
    
    adc_irq_set_enabled(true);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_irq);
    irq_set_enabled(ADC_IRQ_FIFO,true);

    adc_run(true);
}
