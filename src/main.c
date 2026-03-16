#include "stm32f4xx.h"
#include "core_cm4.h"

#include "clock.h"

void main() {
  clock_config();

  // enable FPU
  SCB->CPACR |= (0b11 << 22) | (0b11 <<  20);

  // enable GPIOD peripheral clock
  RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIODEN_Pos);
  
  // set PD12 as output
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER12_Pos);

  volatile float n = 12.4578f;
  float k = n * 0.83f;
  n = n / k;

  GPIOD->ODR |= GPIO_ODR_OD12;

  for (;;) {
    __asm__("nop");
  }
}
