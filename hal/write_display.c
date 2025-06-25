#include <hardware/i2c.h>
#include "ssd1306.h"
#include "pico/stdlib.h"

const uint8_t I2C_SDA = 14;
const uint8_t I2C_SCL = 15;
ssd1306_t disp;

void init_display (void){
    i2c_init(i2c1,400*1000); // 400kHz
    gpio_set_function(I2C_SDA,GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL,GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp);
}

void write_char(char c, size_t word_count, uint8_t x, uint8_t  y, uint8_t  scale)
{
    ssd1306_draw_char(&disp, x, y, scale, c);
}

void write_line (uint8_t x0 , uint8_t y0, uint8_t x1, uint8_t y1)
{
    ssd1306_draw_line(&disp, x0, y0, x1, y1);
}

void write_string (char *c[],uint8_t x, uint8_t  y, uint8_t  scale){
    ssd1306_draw_string(&disp,x,y,scale,c[0]);
}

void draw_bar (uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    ssd1306_draw_square(&disp, x, y, width, height);
}

void show_display(void)
{
    ssd1306_show(&disp);
}

void clear_display(void)
{
    ssd1306_clear(&disp);
}