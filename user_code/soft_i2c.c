#include "soft_i2c.h"

static void i2c_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 80; i++) { __NOP(); }
}

#define SCL_H()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET)
#define SCL_L()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET)
#define SDA_H()  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET)
#define SDA_L()  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET)
#define SDA_IN() HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin)

void i2c_init(void)
{
    SCL_H();
    SDA_H();
}

void i2c_start(void)
{
    SDA_H();
    SCL_H();
    i2c_delay();
    SDA_L();
    i2c_delay();
    SCL_L();
}

void i2c_stop(void)
{
    SDA_L();
    SCL_H();
    i2c_delay();
    SDA_H();
    i2c_delay();
}

uint8_t i2c_write_byte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            SDA_H();
        else
            SDA_L();
        byte <<= 1;
        i2c_delay();
        SCL_H();
        i2c_delay();
        SCL_L();
    }
    SDA_H();
    SCL_H();
    i2c_delay();
    uint8_t ack = SDA_IN();
    SCL_L();
    return ack;
}

