#include "stm32f407xx.h"

#include "clock.h"
#include "pwm.h"

const uint16_t sequence[6] = { 0, 200, 400, 600, 800, 1000 };
volatile uint32_t index = 0;

void EXTI0_IRQHandler() {
  // unpend the interrupt
  EXTI->PR = EXTI_PR_PR0;

  // increment index
  index = (index + 1) % 6;
  // update PWM width
  TIM4->CCR1 = sequence[index];
  TIM4->CCR2 = sequence[index];
  TIM4->CCR3 = sequence[index];
  TIM4->CCR4 = sequence[index];
}

void main() {
  // enable FPU
  SCB->CPACR |= (0b11 << 22) | (0b11 << 20);

  // set the system clock
  clock_init();
  // configure TIM4 for PWM with PD[13..15]
  pwm_init();

  // enable GPIOA peripheral clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

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
  __asm__ volatile("cpsie i");

  for (;;) {
    // wait for a button press
    __asm__ volatile("wfi");
  }
}
