#include "stm32f407xx.h"

#include "pwm.h"

void pwm_init() {
  // enable GPIOD peripheral clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

  // set PD12, PD13, PD14, PD15 to AF mode
  GPIOD->MODER |= (0xAAu << 24); // we have to put the 'u' suffix, fuck the C programming language
                                 // for not making integers cross-platform and not doing type
                                 // coercion on constant expressions
                                 // alternatively we do this:
                                 // GPIOD->MODER |= 0xAA000000;

  // set them as AF2 (TIM4 output)
  GPIOD->AFR[1] |= (0x2222 << 16);

  // enable TIM4 input clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  // set the prescaler 84MHz / 84 = 1MHz
  TIM4->PSC = 84 - 1;
  // set the counter reload 1MHz / 1000 = 1kHz
  TIM4->ARR = 1000 - 1;
  // set PWM mode 1 on output channels
  TIM4->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos) | (0b110 << TIM_CCMR1_OC2M_Pos);
  TIM4->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos) | (0b110 << TIM_CCMR2_OC4M_Pos);
  // set auto-reload of the output compare
  TIM4->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
  TIM4->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
  // enable output channels
  TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

  // set output compare
  TIM4->CCR1 = (uint16_t)0;
  TIM4->CCR2 = (uint16_t)0;
  TIM4->CCR3 = (uint16_t)0;
  TIM4->CCR4 = (uint16_t)0;

  // enable the timer
  TIM4->CR1 |= TIM_CR1_CEN;
}

void pwm_set(uint16_t timestamp) {
  TIM4->CCR1 = timestamp;
  TIM4->CCR2 = timestamp;
  TIM4->CCR3 = timestamp;
  TIM4->CCR4 = timestamp;
}
