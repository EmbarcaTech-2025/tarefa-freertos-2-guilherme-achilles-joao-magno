#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ssd1306.h"
#include "write_display.h"
#include "temperature_sensor.h"
#include "temp_transform.h"
#include "neopixel_pio.h"
#include "visor_dinamica.h"
#include "math.h"

#define TAM_JANELA 10
#define BUTTON_A 5
#define BUTTON_B 6
#define DEBOUNCE_TIME_MS 1000

volatile uint32_t last_interrupt_time_a = 0;
volatile uint32_t last_interrupt_time_b = 0;

// 0 = temperatura, 1 = estatística, 2 = gráfico
BaseType_t modo_display = 0;

char temp_buffer[20];
float temp_atual = 0.0f;

SemaphoreHandle_t xDisplayMutex;
QueueHandle_t xFilaTemperaturas;

// Handles das tasks de display e controle
TaskHandle_t hDisplayTemp;
TaskHandle_t hDisplayEstat;
TaskHandle_t hDisplayGraf;
TaskHandle_t hButtonCtrl;

// Task de controle de botões A e B
void button_control_task(void *params) {
    uint32_t val;
    while (1) {
        if (xTaskNotifyWait(0, 0, &val, portMAX_DELAY) == pdTRUE) {
            switch (val) {
                case 0: // Botão A
                    if (modo_display != 1) {
                        // entrar em estatística
                        modo_display = 1;
                        vTaskSuspend(hDisplayTemp);
                        vTaskSuspend(hDisplayGraf);
                        vTaskResume(hDisplayEstat);
                    } else {
                        // voltar para temperatura
                        modo_display = 0;
                        vTaskSuspend(hDisplayEstat);
                        vTaskSuspend(hDisplayGraf);
                        vTaskResume(hDisplayTemp);
                    }
                    break;
                case 1: // Botão B
                    if (modo_display != 2) {
                        // entrar em gráfico
                        modo_display = 2;
                        vTaskSuspend(hDisplayTemp);
                        vTaskSuspend(hDisplayEstat);
                        vTaskResume(hDisplayGraf);
                    } else {
                        // voltar para temperatura
                        modo_display = 0;
                        vTaskSuspend(hDisplayEstat);
                        vTaskSuspend(hDisplayGraf);
                        vTaskResume(hDisplayTemp);
                    }
                    break;
            }
        }
    }
}

// ISR para ambos os botões
void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t woken = pdFALSE;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BUTTON_A && (now - last_interrupt_time_a) > DEBOUNCE_TIME_MS) {
            last_interrupt_time_a = now;
            xTaskNotifyFromISR(hButtonCtrl, 0, eSetValueWithOverwrite, &woken);
        }
        if (gpio == BUTTON_B && (now - last_interrupt_time_b) > DEBOUNCE_TIME_MS) {
            last_interrupt_time_b = now;
            xTaskNotifyFromISR(hButtonCtrl, 1, eSetValueWithOverwrite, &woken);
        }
    }
    portYIELD_FROM_ISR(woken);
}

// Tarefa de display de temperatura
void display_task_temp(void *params) {
    while (1) {
        if (modo_display == 0) {
            char local[20] = {0};
            if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                strcpy(local, temp_buffer);
                xSemaphoreGive(xDisplayMutex);
            }
            clear_display();
            write_string((char*[]){"Temperatura:"}, 2, 4, 1);
            write_string((char*[]){local}, 30, 20, 2);
            write_string((char*[]){"A=Estat B=Graf"}, 2, 56, 1);
            show_display();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Tarefa de display de estatísticas
void display_task_estat(void *params) {
    float buf[TAM_JANELA] = {0};
    int idx = 0, cnt = 0;
    while (1) {
        if (modo_display == 1) {
            float v;
            if (xQueueReceive(xFilaTemperaturas, &v, 0) == pdTRUE) {
                buf[idx] = v;
                idx = (idx + 1) % TAM_JANELA;
                if (cnt < TAM_JANELA) cnt++;
            }
            float sum = 0;
            for (int i = 0; i < cnt; i++) sum += buf[i];
            float m = cnt ? sum / cnt : 0;
            float sv = 0;
            for (int i = 0; i < cnt; i++) sv += (buf[i] - m) * (buf[i] - m);
            float var = cnt ? sv / cnt : 0;
            float dp = sqrtf(var);
            clear_display();
            char t1[20], t2[20], t3[20];
            sprintf(t1, "Media: %.2f", m);
            sprintf(t2, "DP: %.2f", dp);
            sprintf(t3, "Var: %.2f", var);
            write_string((char*[]){t1}, 10, 16, 1);
            write_string((char*[]){t2}, 10, 32, 1);
            write_string((char*[]){t3}, 10, 48, 1);
            show_display();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Tarefa de display de gráfico
void display_task_graf(void *params) {
    float buf[TAM_JANELA] = {0};
    int idx = 0, cnt = 0;
    while (1) {
        if (modo_display == 2) {
            float v;
            if (xQueueReceive(xFilaTemperaturas, &v, 0) == pdTRUE) {
                buf[idx] = v;
                idx = (idx + 1) % TAM_JANELA;
                if (cnt < TAM_JANELA) cnt++;
            }
            clear_display();
            int nb = cnt < 6 ? cnt : 6;
            for (int i = 0; i < nb; i++) {
                int pos = (idx - nb + i + TAM_JANELA) % TAM_JANELA;
                int h_val = (int)buf[pos];
                if (h_val > 64) h_val = 64;
                draw_bar(20 + 12 * i, 64 - h_val, 8, 64);
            }
            show_display();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Leitura do sensor de temperatura
void temp_read_task(void *params) {
    while (1) {
        float v = get_voltage();
        float t = adc_to_celsius(v);
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
            temp_atual = t;
            sprintf(temp_buffer, "%.2fC", t);
            xSemaphoreGive(xDisplayMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Envio para fila
void fila_task(void *params) {
    while (1) {
        float lt;
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
            lt = temp_atual;
            xSemaphoreGive(xDisplayMutex);
            xQueueSend(xFilaTemperaturas, &lt, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void dinamic_matriz_task(void *params) {
    float local_temp;

    while (true) {
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
            local_temp = temp_atual;
            xSemaphoreGive(xDisplayMutex);
        }

        printf("Temperatura atual: %.2f C\n", local_temp);
        matriz_dinamic(local_temp);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main() {
    stdio_init_all();
    init_display(); init_temp(); init_fifo_temp(); npInit(7, 25); clear_display();
    gpio_init(BUTTON_A); gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    xFilaTemperaturas = xQueueCreate(10, sizeof(float));
    xDisplayMutex = xSemaphoreCreateMutex();

    // Criar tasks
    xTaskCreate(temp_read_task, "temp_read", 256, NULL, 1, NULL);
    xTaskCreate(fila_task, "fila", 256, NULL, 2, NULL);
    xTaskCreate(display_task_temp, "disp_temp", 512, NULL, 3, &hDisplayTemp);
    xTaskCreate(display_task_estat, "disp_estat", 512, NULL, 3, &hDisplayEstat);
    xTaskCreate(display_task_graf, "disp_graf", 512, NULL, 3, &hDisplayGraf);
    xTaskCreate(button_control_task, "btn_ctrl", 256, NULL, 4, &hButtonCtrl);
    xTaskCreate(dinamic_matriz_task, "dinamic_matriz", 256, NULL, 3, NULL);

    // Iniciar com apenas modo temperatura
    modo_display = 0;
    vTaskSuspend(hDisplayEstat);
    vTaskSuspend(hDisplayGraf);

    vTaskStartScheduler();
    while (1);
}
