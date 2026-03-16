#include "stm32f4xx.h"

#include "clock.h"

void clock_config() {
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

  return;
}
