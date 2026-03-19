#include "stm32f407xx.h"

#include "systick.h"

// counts milliseconds from enabling the SysTick timer
volatile uint32_t epoch = 0;

// SysTick IRQ Handler
void SysTick_Handler() {
  epoch += 1;
}

void systick_init() {
  // configure SysTick to trigger every 1kHz (168MHz / 168000)
  SysTick->LOAD = 168000 - 1;
  // set SysTick IRQ priority
  SCB->SHPR[11] = (0b0111 << 4);
  // reset counter value to 0
  SysTick->VAL = 0;
  // select the source clock to be sysclk, enable the timer and its interrupt
  SysTick->CTRL |= (1 << SysTick_CTRL_CLKSOURCE_Pos) | (1 << SysTick_CTRL_TICKINT_Pos) |
                   (1 << SysTick_CTRL_ENABLE_Pos);
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
