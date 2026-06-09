#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include "main.h"

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write_byte(uint8_t byte);

#endif


