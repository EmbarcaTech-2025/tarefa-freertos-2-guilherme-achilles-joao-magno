#ifndef WRITE_DISPLAY_H
#define WRITE_DISPLAY_H

#include "ssd1306.h"

void init_display(void);
void write_char(char c, size_t word_count, uint8_t  x, uint8_t y, uint8_t scale);
void write_line (uint8_t x0 , uint8_t y0, uint8_t x1, uint8_t y1);
void write_string (char *c,uint8_t x, uint8_t  y, uint8_t  scale);
void show_display(void);
void clear_display(void);
void draw_bar (uint8_t x, uint8_t y, uint8_t width, uint8_t height);

#endif


