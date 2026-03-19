#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

// Configure the SysTick timer
void systick_init();

// Delay the program by `time` milliseconds using the SysTick timer
void delay_ms(uint32_t time);

#endif
