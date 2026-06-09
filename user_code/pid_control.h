#ifndef __PID_CONTROL_H
#define __PID_CONTROL_H

#include <stdint.h>

//PID控制器结构体
typedef struct {
    float kp;
    float ki;
    float kd;
	
    float sp;             //目标值
    float last_err;       //上一次误差 e(k-1)
    float prev_err;       //上上次误差 e(k-2)

    int32_t output;       //当前输出值
    int32_t out_min;      
    int32_t out_max;
} pid_t;

void pid_init(pid_t *pid, float kp, float ki, float kd, int32_t out_max);

//设置目标值
void pid_setpoint(pid_t *pid, float sp);	

int32_t pid_compute(pid_t *pid, float measurement);

//重置
void pid_reset(pid_t *pid);

#endif

