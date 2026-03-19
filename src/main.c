#include "stm32f407xx.h"

#include "clock.h"
#include "pwm.h"
#include "spi.h"
#include "systick.h"

volatile int16_t x;
volatile int16_t y;
volatile int16_t z;

void main() {
  // enable FPU
  SCB->CPACR |= (0b11 << 22) | (0b11 << 20);

  // set the system clock
  clock_init();
  // initialise the SysTick timer
  systick_init();
  // configure TIM4 for PWM with PD[13..15]
  pwm_init();
  // configure SPI1 for use with the LIS3DSH sensor
  spi_init();

  // enable interrupts
  __asm__ volatile("cpsie i");

  // configure the bandwidth and enable axes
  spi_write(0x20, 0x6F);
  spi_write(0x24, 0x00);

  uint8_t buf[6];

  for (;;) {
    for (uint8_t i = 0; i < 6; i++) {
      buf[i] = spi_read(0x28 + i);
    }

    x = (int16_t)((buf[1] << 8) | buf[0]);
    y = (int16_t)((buf[3] << 8) | buf[2]);
    z = (int16_t)((buf[5] << 8) | buf[4]);

    delay_ms(500);
  }
}
