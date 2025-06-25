#include <stdint.h>

float adc_to_celsius (float tensao){
    float temperature = 27 - ((tensao - 0.706)/ 0.001721f); // Converte a tensão para temperatura em Celsius
    return temperature; // Retorna a temperatura em Celsius
}
