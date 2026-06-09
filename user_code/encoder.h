#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"

void     enc_init(void);
int16_t  enc_get_delta(void);
uint8_t  enc_button_pressed(void);
uint8_t  enc_button_long_press(void);
void     enc_timer_tick(void);
void     enc_exti_callback(uint16_t GPIO_Pin);

#endif


