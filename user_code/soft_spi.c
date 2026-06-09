#include "soft_spi.h"

#define CS_LOW()   HAL_GPIO_WritePin(MAX6675_CS_GPIO_Port, MAX6675_CS_Pin, GPIO_PIN_RESET)
#define CS_HIGH()  HAL_GPIO_WritePin(MAX6675_CS_GPIO_Port, MAX6675_CS_Pin, GPIO_PIN_SET)
#define SCK_LOW()  HAL_GPIO_WritePin(MAX6675_SCK_GPIO_Port, MAX6675_SCK_Pin, GPIO_PIN_RESET)
#define SCK_HIGH() HAL_GPIO_WritePin(MAX6675_SCK_GPIO_Port, MAX6675_SCK_Pin, GPIO_PIN_SET)
#define MISO_GET() HAL_GPIO_ReadPin(MAX6675_MISO_GPIO_Port, MAX6675_MISO_Pin)

void soft_spi_init(void)
{
    CS_HIGH();
    SCK_LOW();
}

void soft_spi_cs_low(void)
{
    CS_LOW();
}

void soft_spi_cs_high(void)
{
    CS_HIGH();
}

uint8_t soft_spi_read_byte(void)
{
    uint8_t data = 0;
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        SCK_HIGH();
        data <<= 1;
        if (MISO_GET())
        {
            data |= 0x01;
        }
        SCK_LOW();
    }
    return data;
}


