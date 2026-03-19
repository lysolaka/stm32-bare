#ifndef PWM_H
#define PWM_H

#include <stdint.h>

// Configure TIM4 for PWM with outputs on PD[12..15]
void pwm_init();

// Set the PWM duty to `timestamp` for a specific channel.
// The value of `1000` is 100%, `0` is 0%.
// `channel` must be in the range [0..4]
void pwm_set(uint16_t timestamp, uint8_t channel);

#endif
