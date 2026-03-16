#include "stm32f4xx.h"

#include "clock.h"

void main() {
  // enable FPU
  SCB->CPACR |= (0b11 << 22) | (0b11 << 20);

  clock_init();
  __enable_irq();

  // enable GPIOD peripheral clock
  RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIODEN_Pos);

  // set PD12 as output
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER12_Pos);
  // set PD13 as output
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER13_Pos);

  volatile float n = 12.4578f;
  float k = n * 0.83f;
  n = n / k;

  GPIOD->ODR |= GPIO_ODR_OD12;

  for (uint32_t i = 20; i > 0; i--) {
    GPIOD->ODR ^= GPIO_ODR_OD13;
    delay_ms(500);
  }

  GPIOD->ODR |= GPIO_ODR_OD13;

  for (;;) {
    GPIOD->ODR ^= GPIO_ODR_OD12;
    delay_ms(500);
  }
}
