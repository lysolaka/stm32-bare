#include "stm32f407xx.h"

#include "clock.h"
#include "pwm.h"
#include "spi.h"
#include "systick.h"

uint16_t calc_timestamp(int16_t a);

void EXTI0_IRQHandler() {
  // unpend the interrupt
  EXTI->PR = EXTI_PR_PR0;

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
    TIM4->CCR1 = xi;
    TIM4->CCR3 = 0;
  } else {
    TIM4->CCR1 = 0;
    TIM4->CCR3 = xi;
  }

  // same for Y
  uint16_t yi = calc_timestamp(y);
  if (y < 0) {
    TIM4->CCR2 = 0;
    TIM4->CCR4 = yi;
  } else {
    TIM4->CCR2 = yi;
    TIM4->CCR4 = 0;
  }
}

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

  // enable GPIOE peripheral clock (already enabled but doesn't hurt to do it again)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
  // enable the SYSCFG peripheral clock
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  // set PE0 as pull-down input
  GPIOE->PUPDR |= (0b10 << GPIO_PUPDR_PUPD0_Pos);
  // select PE0 as EXTI0 input
  SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PE;
  // unmask EXTI0 interrupt
  EXTI->IMR |= EXTI_IMR_MR0;
  // trigger on rising edge
  EXTI->RTSR |= EXTI_RTSR_TR0;
  // set a priority
  NVIC->IPR[EXTI0_IRQn] = (0b0110 << 4);
  // enable EXTI line 0 interrupt in the NVIC
  NVIC->ISER[EXTI0_IRQn >> 5] = (1 << (EXTI0_IRQn & 0x1F));

  // enable interrupts
  __asm__ volatile("cpsie i");

  // configure the bandwidth and enable axes
  spi_write(0x20, 0x63);
  // enable DRDY interrupt
  spi_write(0x23, 0xC8);

  for (;;) {
    __asm__ volatile("wfi");
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

  // for values above 1.5g just put the PWM timestamp to the maximum
  if (v >= 24576)
    return 1000;

  // compute the value bin index
  // equivalent to floorf(v / 2457.6f) using integer arithmetic for 100 discrete bins
  int32_t bin = (v * 100) / 24576;

  // multiply the bin index by the increment value
  return (uint16_t)(bin * 10);
}
