#include "stm32f4xx.h"

#include "clock.h"

// counts milliseconds from enabling the SysTick timer
volatile uint32_t epoch = 0;

void clock_init() {
  // enable HSE
  RCC->CR |= RCC_CR_HSEON;
  // wait for it to start
  while (!(RCC->CR & RCC_CR_HSERDY_Msk))
    ;

  // PLL values:
  // M = 4, N = 168, P = 2 (reset value), Q = 8

  // clear the register
  RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLQ_Msk | RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLM_Msk);
  // apply the configuration, use HSE for the PLL
  RCC->PLLCFGR |= (8 << RCC_PLLCFGR_PLLQ_Pos) | (168 << RCC_PLLCFGR_PLLN_Pos) |
                  (4 << RCC_PLLCFGR_PLLM_Pos) | RCC_PLLCFGR_PLLSRC_HSE;

  // prescale APB1 by 4, APB2 by 2
  RCC->CFGR |= (0b101 << RCC_CFGR_PPRE1_Pos) | (0b100 << RCC_CFGR_PPRE2_Pos);

  // enable PLL
  RCC->CR |= (1 << RCC_CR_PLLON_Pos);
  // wait for it to start
  while (!(RCC->CR & RCC_CR_PLLRDY_Msk))
    ;

  // configure flash latency
  FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;

  // set PLL as the system clock
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  // wait for the switch
  while (!(RCC->CFGR & RCC_CFGR_SWS_PLL))
    ;

  // configure SysTick
  SysTick->LOAD = (uint32_t)(168000 - 1);
  // set SysTick IRQ priority
  __NVIC_SetPriority(SysTick_IRQn, 0b0111);
  // reset counter value to 0
  SysTick->VAL = (uint32_t)0;
  // select the source clock to be sysclk, enable the timer and its interrupt
  SysTick->CTRL |= (1 << SysTick_CTRL_CLKSOURCE_Pos) | (1 << SysTick_CTRL_TICKINT_Pos) |
                   (1 << SysTick_CTRL_ENABLE_Pos);

  return;
}

void delay_ms(uint32_t time) {
  // capture the epoch and calculate the wait time
  uint32_t start = epoch;
  uint32_t end = start + time;

  // if overflow wait a little more
  if (end < start) {
    while (time > start)
      ;
  }

  // wait for the time to come
  while (epoch < end)
    ;
}

void SysTick_Handler() {
  epoch += 1;
}
