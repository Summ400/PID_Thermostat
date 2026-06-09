#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>

#define FONT_WIDTH   8
#define FONT_HEIGHT  16
#define FONT_START   32
#define FONT_END     126

extern const uint8_t font8x16[][16];

#endif
