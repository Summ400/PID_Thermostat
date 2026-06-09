#include "pid_control.h"

void pid_init(pid_t *pid, float kp, float ki, float kd, int32_t out_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->sp = 0.0f;
    pid->last_err = 0.0f;
    pid->prev_err = 0.0f;
    pid->output = 0;
    pid->out_min = 0;
    pid->out_max = out_max;
}

//设置目标值
void pid_setpoint(pid_t *pid, float sp)
{
    pid->sp = sp;
}

int32_t pid_compute(pid_t *pid, float measurement)
{
	float error = pid->sp - measurement;
	
	float p_term = pid->kp * (error - pid->last_err);
    float i_term = pid->ki * error;
    float d_term = pid->kd * (error - 2.0f * pid->last_err + pid->prev_err);
	
	int32_t delta = (int32_t)(p_term + i_term + d_term);
	
	pid->output += delta;
	
    //输出限幅，防止溢出
    if (pid->output > pid->out_max)
        pid->output = pid->out_max;
    if (pid->output < pid->out_min)
        pid->output = pid->out_min;
	
    pid->prev_err = pid->last_err;
    pid->last_err = error;

    return pid->output;
}


void pid_reset(pid_t *pid)
{
    pid->last_err = 0.0f;
    pid->prev_err = 0.0f;
    pid->output = 0;
}


