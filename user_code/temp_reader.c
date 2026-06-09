#include "temp_reader.h"
#include "soft_spi.h"

void temp_init(void)
{
    soft_spi_init();
}

static uint16_t read_max6675(void)
{
    uint16_t data = 0;

    soft_spi_cs_low();
    data = spi_read_byte();
    data <<= 8;
    data |= spi_read_byte();
    soft_spi_cs_high();

    return data;
}

void temp_read_all(float *temp, uint8_t *tc_ok)
{
    uint16_t raw = read_max6675();

    *tc_ok = (raw & 0x04) ? 0 : 1;

    raw >>= 3;
    *temp = (float)raw * 0.25f;
}
