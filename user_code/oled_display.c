#include "oled_display.h"
#include "soft_i2c.h"
#include "oled_font.h"

static void oled_write_cmd(uint8_t cmd)
{
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x00);
    i2c_write_byte(cmd);
    i2c_stop();
}

static void oled_write_data(uint8_t data)
{
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x40);
    i2c_write_byte(data);
    i2c_stop();
}

static void oled_set_page(uint8_t page)
{
    oled_write_cmd(0xB0 + (page & 0x07));
}

static void oled_set_col(uint8_t col)
{
    oled_write_cmd(0x00 + (col & 0x0F));
    oled_write_cmd(0x10 + ((col >> 4) & 0x0F));
}

static void oled_set_pos(uint8_t page, uint8_t col)
{
    oled_set_page(page);
    oled_set_col(col);
}

void oled_init(void)
{
    volatile uint32_t d;
    for (d = 0; d < 100000; d++) { __NOP(); }

    oled_write_cmd(0xAE);
    oled_write_cmd(0x20); oled_write_cmd(0x00);
    oled_write_cmd(0x40);
    oled_write_cmd(0xA1);
    oled_write_cmd(0xC8);
    oled_write_cmd(0xA8); oled_write_cmd(0x1F);
    oled_write_cmd(0xD3); oled_write_cmd(0x00);
    oled_write_cmd(0xD5); oled_write_cmd(0x80);
    oled_write_cmd(0xD9); oled_write_cmd(0xF1);
    oled_write_cmd(0xDA); oled_write_cmd(0x02);
    oled_write_cmd(0xDB); oled_write_cmd(0x40);
    oled_write_cmd(0x8D); oled_write_cmd(0x14);
    oled_write_cmd(0xA4);
    oled_write_cmd(0xA6);
    oled_write_cmd(0xAF);
}

void oled_clear(void)
{
    uint8_t p, c;
    for (p = 0; p < 4; p++)
    {
        oled_set_pos(p, 0);
        for (c = 0; c < 128; c++)
        {
            oled_write_data(0x00);
        }
    }
}

static void oled_show_char_font(uint8_t x, uint8_t line, char ch)
{
    uint8_t idx;
    if (ch >= FONT_START && ch <= FONT_END)
        idx = ch - FONT_START;
    else
        idx = 0;

    uint8_t i;
    /* upper half */
    oled_set_pos(line * 2, x);
    for (i = 0; i < 8; i++)
        oled_write_data(font8x16[idx][i]);
    /* lower half */
    oled_set_pos(line * 2 + 1, x);
    for (i = 0; i < 8; i++)
        oled_write_data(font8x16[idx][i + 8]);
}

void oled_show_string(uint8_t x, uint8_t line, const char *str)
{
    while (*str)
    {
        oled_show_char_font(x, line, *str);
        x += FONT_WIDTH;
        if (x + FONT_WIDTH > 128)
            break;
        str++;
    }
}

void oled_show_number(uint8_t x, uint8_t line, float num, uint8_t decimals)
{
    char buf[12];
    int int_part = (int)num;
    int frac;
    int len = 0;

    if (num < 0)
    {
        int_part = -int_part;
        buf[len++] = '-';
    }

    if (int_part == 0)
    {
        buf[len++] = '0';
    }
    else
    {
        char tmp[6];
        int t = 0;
        while (int_part > 0)
        {
            tmp[t++] = '0' + (int_part % 10);
            int_part /= 10;
        }
        while (t > 0)
            buf[len++] = tmp[--t];
    }

    if (decimals > 0)
    {
        buf[len++] = '.';
        if (num < 0) num = -num;
        frac = (int)((num - (int)num) * 10 + 0.5f);
        buf[len++] = '0' + (frac % 10);
    }

    buf[len] = '\0';
    oled_show_string(x, line, buf);
}


