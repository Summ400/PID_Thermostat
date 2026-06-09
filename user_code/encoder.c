#include "encoder.h"

static volatile int16_t  enc_delta = 0;

#define ENC_DEBOUNCE_MS        20
#define ENC_LONG_PRESS_MS      2000

typedef enum {
    ENC_BTN_IDLE = 0,
    ENC_BTN_DEBOUNCE,
    ENC_BTN_PRESSED,
    ENC_BTN_RELEASE_DEBOUNCE
} enc_btn_state_t;

static volatile enc_btn_state_t btn_state = ENC_BTN_IDLE;
static volatile uint16_t       btn_timer = 0;
static volatile uint8_t        btn_short_flag = 0;
static volatile uint8_t        btn_long_flag = 0;
static volatile uint8_t        btn_long_fired = 0;

void enc_init(void)
{
    /* PA11/PA12已由CubeMX配置为EXTI，无需额外初始化 */
}

int16_t enc_get_delta(void)
{
    int16_t d = enc_delta;
    enc_delta = 0;
    return d;
}

uint8_t enc_button_pressed(void)
{
    if (btn_short_flag)
    {
        btn_short_flag = 0;
        return 1;
    }
    return 0;
}

uint8_t enc_button_long_press(void)
{
    if (btn_long_flag)
    {
        btn_long_flag = 0;
        return 1;
    }
    return 0;
}

void enc_timer_tick(void)
{
    switch (btn_state)
    {
    case ENC_BTN_IDLE:
        if (HAL_GPIO_ReadPin(EC11_SW_GPIO_Port, EC11_SW_Pin) == GPIO_PIN_RESET)
        {
            btn_state = ENC_BTN_DEBOUNCE;
            btn_timer = 0;
        }
        break;

    case ENC_BTN_DEBOUNCE:
        btn_timer++;
        if (btn_timer >= ENC_DEBOUNCE_MS)
        {
            if (HAL_GPIO_ReadPin(EC11_SW_GPIO_Port, EC11_SW_Pin) == GPIO_PIN_RESET)
            {
                btn_state = ENC_BTN_PRESSED;
                btn_timer = 0;
                btn_long_fired = 0;
            }
            else
            {
                btn_state = ENC_BTN_IDLE;
            }
        }
        break;

    case ENC_BTN_PRESSED:
        btn_timer++;
        if (!btn_long_fired && btn_timer >= ENC_LONG_PRESS_MS)
        {
            btn_long_flag = 1;
            btn_long_fired = 1;
        }
        if (HAL_GPIO_ReadPin(EC11_SW_GPIO_Port, EC11_SW_Pin) == GPIO_PIN_SET)
        {
            btn_state = ENC_BTN_RELEASE_DEBOUNCE;
            btn_timer = 0;
        }
        break;

    case ENC_BTN_RELEASE_DEBOUNCE:
        btn_timer++;
        if (btn_timer >= ENC_DEBOUNCE_MS)
        {
            if (HAL_GPIO_ReadPin(EC11_SW_GPIO_Port, EC11_SW_Pin) == GPIO_PIN_SET)
            {
                if (!btn_long_fired)
                {
                    btn_short_flag = 1;
                }
                btn_state = ENC_BTN_IDLE;
            }
            else
            {
                btn_state = ENC_BTN_PRESSED;
                btn_timer = 0;
            }
        }
        break;
    }
}

void enc_exti_callback(uint16_t GPIO_Pin)
{
    uint8_t a_level = HAL_GPIO_ReadPin(EC11_A_GPIO_Port, EC11_A_Pin);
    uint8_t b_level = HAL_GPIO_ReadPin(EC11_B_GPIO_Port, EC11_B_Pin);

    if (GPIO_Pin == EC11_A_Pin)
    {
        if (a_level == b_level)
            enc_delta--;
        else
            enc_delta++;
    }
    else if (GPIO_Pin == EC11_B_Pin)
    {
        if (a_level != b_level)
            enc_delta--;
        else
            enc_delta++;
    }
}

