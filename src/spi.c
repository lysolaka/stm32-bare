#include "stm32f407xx.h"

#include "spi.h"

void spi_init() {
  // enable GPIO{A,E} peripheral clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOEEN;

  // set PE3 to push-pull output (is used as CS for SPI)
  GPIOE->MODER |= (0b01 << GPIO_MODER_MODER3_Pos);
  // set PE3 to high (CS is active low)
  GPIOE->BSRR = GPIO_BSRR_BS3;

  // set PA[5..7] to AF mode
  GPIOA->MODER |= (0b10 << GPIO_MODER_MODER5_Pos) | (0b10 << GPIO_MODER_MODER6_Pos) |
                  (0b10 << GPIO_MODER_MODER7_Pos);
  // make them (very) fast
  GPIOA->OSPEEDR |= (0xFC << 8);
  // set them to AF5 (SPI1)
  GPIOA->AFR[0] |= (0x555 << 20);

  // enable SPI1 peripheral clock
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

  // set the baud rate to f_PCLK / 16
  // set proper clock polarity and capture edge
  // set software CS management
  // set the device as master
  SPI1->CR1 |= (0b011 << SPI_CR1_BR_Pos) | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM | SPI_CR1_SSI |
               SPI_CR1_MSTR;

  // enable the SPI
  SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t spi_transaction(uint8_t addr, uint8_t tx) {
  // assert chip select (active low)
  GPIOE->BSRR = GPIO_BSRR_BR3;
  // wait for TX to be ready
  while (!(SPI1->SR & SPI_SR_TXE))
    ;

  // send an address with the read bit set
  SPI1->DR = addr;
  // wait for reception
  while (!(SPI1->SR & SPI_SR_RXNE))
    ;

  // discard the result
  (void)SPI1->DR;

  // wait for TX to be ready
  while (!(SPI1->SR & SPI_SR_TXE))
    ;

  // send out data
  SPI1->DR = tx;
  // wait for reception
  while (!(SPI1->SR & SPI_SR_RXNE))
    ;

  // store the received data
  uint8_t rx = *((uint8_t*)&SPI1->DR);
  // wait for the transaction to finish
  while (SPI1->SR & SPI_SR_BSY)
    ;

  // deassert chip select
  GPIOE->BSRR = GPIO_BSRR_BS3;

  return rx;
}

uint8_t spi_read(uint8_t addr) {
  // set the read bit in the address, send out dummy 0s to keep the SCK running
  return spi_transaction(addr | 0x80, 0x00);
}

void spi_write(uint8_t addr, uint8_t data) {
  // set the read bit to 0
  spi_transaction(addr & 0x7F, data);
}
