#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

// Confugure the system clock
void clock_init();

// Blocking delay in milliseconds
void delay_ms(uint32_t time);

#endif
