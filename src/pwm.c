#include "stm32f407xx.h"

#include "pwm.h"

void pwm_init() {
  // enable GPIOD peripheral clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

  // set PD[12..15] to AF mode
  // for PD15 we have to put the 'u' suffix since we put a 1 in the MSB, which would change the
  // sign, fuck the C programming language for not making integers cross-platform and not doing type
  // coercion on constant expressions
  GPIOD->MODER |= (0b10 << GPIO_MODER_MODER12_Pos) | (0b10 << GPIO_MODER_MODER13_Pos) |
                  (0b10 << GPIO_MODER_MODER14_Pos) | (0b10u << GPIO_MODER_MODER15_Pos);

  // set them as AF2 (TIM4 output)
  GPIOD->AFR[1] |= (0b0010 << GPIO_AFRH_AFSEL12_Pos) | (0b0010 << GPIO_AFRH_AFSEL13_Pos) |
                   (0b0010 << GPIO_AFRH_AFSEL14_Pos) | (0b0010 << GPIO_AFRH_AFSEL15_Pos);

  // enable TIM4 input clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  // set the prescaler 84MHz / 4 = 21MHz
  TIM4->PSC = 4 - 1;
  // set the counter reload 21MHz / 1000 = 21kHz
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
  TIM4->CCR1 = 0;
  TIM4->CCR2 = 0;
  TIM4->CCR3 = 0;
  TIM4->CCR4 = 0;

  // enable the timer
  TIM4->CR1 |= TIM_CR1_CEN;
}

void pwm_set(uint16_t timestamp, uint8_t channel) {
  // check for a proper channel
  if (channel > 0 && channel < 5) {
    // pointer arithmetic to get the proper channel register
    // the CCR registers are adjacent
    volatile uint32_t* addr = &TIM4->CCR1 + (channel - 1);
    // write the timestamp
    *addr = timestamp;
  }
}
