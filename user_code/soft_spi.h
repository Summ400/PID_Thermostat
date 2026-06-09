#ifndef __SOFT_SPI_H
#define __SOFT_SPI_H

#include "main.h"

void    soft_spi_init(void);
uint8_t spi_read_byte(void);
void    soft_spi_cs_low(void);
void    soft_spi_cs_high(void);

#endif

