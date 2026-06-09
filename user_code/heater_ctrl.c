#include "heater_ctrl.h"

extern TIM_HandleTypeDef htim17;

void heater_init(void)
{
    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, 0);
}

void heater_set_duty(uint16_t duty)
{
    if (duty > HEATER_MAX_DUTY)
        duty = HEATER_MAX_DUTY;

    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, duty);
}

uint16_t heater_get_duty(void)
{
    return __HAL_TIM_GET_COMPARE(&htim17, TIM_CHANNEL_1);
}

void heater_off(void)
{
    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, 0);
}

uint8_t heater_is_safe(float current_temp, uint8_t tc_ok)
{
    if (!tc_ok)
        return 0;
    if (current_temp >= HEATER_SAFE_TEMP)
        return 0;

    return 1;
}











































