#ifndef __HEATER_CTRL_H
#define __HEATER_CTRL_H

#include "main.h"

#define HEATER_MAX_DUTY  65535

#define HEATER_SAFE_TEMP 120.0f

void heater_init(void);
void heater_set_duty(uint16_t duty);
uint16_t heater_get_duty(void);
void heater_off(void);
uint8_t  heater_is_safe(float current_temp, uint8_t tc_ok);

#endif

