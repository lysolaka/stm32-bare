#include "stm32f407xx.h"

#include "clock.h"
#include "pwm.h"
#include "spi.h"
#include "systick.h"

volatile int16_t x, y, z;

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

  // enable interrupts
  __asm__ volatile("cpsie i");

  pwm_set(200, 1);
  pwm_set(400, 2);
  pwm_set(600, 3);
  pwm_set(800, 4);

  // configure the bandwidth and enable axes
  spi_write(0x20, 0x6F);
  spi_write(0x24, 0x00);

  // NOTE: static because we need a const address for DMA
  // read burst with auto-increment
  static uint8_t tx[7] = {0x28 | 0x80};
  // data received (1st byte is garbage)
  static uint8_t rx[7];

  for (;;) {
    spi_dma_transfer(tx, rx, 7);

    __asm__ volatile("wfi");

    x = (int16_t)((rx[2] << 8) | rx[1]);
    y = (int16_t)((rx[4] << 8) | rx[3]);
    z = (int16_t)((rx[6] << 8) | rx[5]);

    delay_ms(10);
  }
}
