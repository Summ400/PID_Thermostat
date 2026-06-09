#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#include "main.h"

#define OLED_ADDR  0x78

void oled_init(void);
void oled_clear(void);
void oled_show_string(uint8_t x, uint8_t line, const char *str);
void oled_show_number(uint8_t x, uint8_t line, float num, uint8_t decimals);

#endif

