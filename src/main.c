#include "stm32f407xx.h"

#include "clock.h"
#include "pwm.h"
#include "spi.h"
#include "systick.h"

uint16_t calc_timestamp(int16_t a);

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
  // configure DMA for SPI1
  spi_dma_init();

  // check if we are talking to the device we expect
  if (spi_read(0x0F) != 0x3F) {
    return;
  }

  // configure the bandwidth and enable axes
  spi_write(0x20, 0x63);
  // enable DRDY interrupt
  // spi_write(0x23, 0xC8);

  // enable interrupts
  __asm__ volatile("cpsie i");

  for (;;) {
    // wait for data to be ready
    while (!(spi_read(0x27) & (1 << 3)))
      ;

    uint8_t buf[4];

    // read 4 bytes X_L, X_H, Y_L and Y_H
    for (uint8_t i = 0; i < 4; i++) {
      buf[i] = spi_read(0x28 + i);
    }

    // reconstruct the int16_t
    int16_t x = (int16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
    int16_t y = (int16_t)((uint16_t)buf[2] | ((uint16_t)buf[3] << 8));

    // set the X axis PWM value in a proper direction
    uint16_t xi = calc_timestamp(x);
    if (x < 0) {
      pwm_set(xi, 1);
      pwm_set(0, 3);
    } else {
      pwm_set(0, 1);
      pwm_set(xi, 3);
    }

    // same for Y
    uint16_t yi = calc_timestamp(y);
    if (y < 0) {
      pwm_set(yi, 4);
      pwm_set(0, 2);
    } else {
      pwm_set(0, 4);
      pwm_set(yi, 2);
    }
  }
}

uint16_t calc_timestamp(int16_t a) {
  // take absolute value
  int32_t v = a;
  if (v < 0)
    v = -v;

  // clamp to int16_t max
  if (v > 32767)
    v = 32767;

  // for values above 1g just put the PWM timestamp to the maximum
  if (v >= 16384)
    return 1000;

  // compute the value bin index
  // equivalent to floorf(v / 1638.4f) using integer arithmetic for 50 discrete bins
  int32_t bin = (v * 50) / 16384;

  // multiply the bin index by the increment value
  return (uint16_t)(bin * 20);
}
