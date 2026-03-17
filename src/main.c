#include "stm32f407xx.h"

#include "clock.h"

volatile uint32_t pattern = 0b0001;

void EXTI0_IRQHandler() {
  // unpend the interrupt
  EXTI->PR = EXTI_PR_PR0;
  // advance the pattern
  pattern = ((pattern << 1) | (pattern >> 3)) & 0xF;
}

void main() {
  // enable FPU
  SCB->CPACR |= (0b11 << 22) | (0b11 << 20);

  clock_init();

  // enable GPIO{A,D} peripheral clocks
  RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOAEN_Pos) | (1 << RCC_AHB1ENR_GPIODEN_Pos);

  // set PD12, PD13, PD14, PD15 as outputs
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER12_Pos);
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER13_Pos);
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER14_Pos);
  GPIOD->MODER |= (0b01 << GPIO_MODER_MODER15_Pos);

  // set PA0 as pull-down input
  GPIOA->PUPDR |= (0b10 << GPIO_PUPDR_PUPD0_Pos);
  // select PA0 as EXTI0 input (reset value)
  // SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
  // unmask EXTI0 interrupt
  EXTI->IMR |= EXTI_IMR_MR0;
  // trigger on rising edge
  EXTI->RTSR |= EXTI_RTSR_TR0;
  // set a priority
  NVIC->IPR[EXTI0_IRQn] = (0b0110 << 4);
  // enable EXTI line 0 interrupt in the NVIC
  NVIC->ISER[0] = (1 << 6);

  // enable interrupts
  __asm__ volatile ("cpsie i");

  for (;;) {
    // clear the output
    GPIOD->ODR &= ~(GPIO_ODR_OD12 | GPIO_ODR_OD13 | GPIO_ODR_OD14 | GPIO_ODR_OD15);
    // set the pattern
    GPIOD->ODR |= (pattern << GPIO_ODR_OD12_Pos);
    // wait for a button press
    __WFI();
  }
}
