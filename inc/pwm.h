#ifndef PWM_H
#define PWM_H

#include <stdint.h>

// Configure TIM4 for PWM with outputs on PD[12..15]
void pwm_init();

// Set the PWM duty to `timestamp`.
// The value of `1000` is 100%, `0` is 0%.
void pwm_set(uint16_t timestamp);

#endif
