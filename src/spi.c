#include "stm32f407xx.h"

#include "spi.h"

void DMA2_Stream0_IRQHandler() {
  // check if it's the interrupt we care about (transfer complete)
  if (DMA2->LISR & DMA_LISR_TCIF0) {
    // clear the interrupt
    DMA2->LIFCR = DMA_LIFCR_CTCIF0;

    // deassert chip select
    GPIOE->BSRR = GPIO_BSRR_BS3;

    // call the user callback
    spi_dma_complete();
  }
}

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

void spi_dma_init() {
  // enable DMA2 peripheral clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  // NOTE: not needed since this is the first time we're configuring the DMA
  // disable DMA2S2 (prepare for configuration)
  // DMA2_Stream0->CR &= ~DMA_SxCR_EN;
  // wait for the channel to be reset
  // while (DMA2_Stream0->CR & DMA_SxCR_EN)
  //   ;

  // select channel 3 (SPI_RX for stream 0)
  // auto-increment memory address
  // enable interrupt on transfer complete
  // direction: peripheral to memory (default)
  DMA2_Stream0->CR = (0b011 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC | DMA_SxCR_TCIE;

  // set peripheral address to the SPI1 Data Register
  DMA2_Stream0->PAR = (uint32_t)&SPI1->DR;

  // see the note above
  // DMA2_Stream3->CR = 0;
  // while (DMA2_Stream3->CR & DMA_SxCR_EN)
  //   ;

  // seelect channel 3 (SPI_TX for stream 3)
  // auto-increment memory address
  // direction: memory to peripheral
  DMA2_Stream3->CR = (0b011 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC | (0b01 << DMA_SxCR_DIR_Pos);

  // set peripheral address to the SPI1 Data Register
  DMA2_Stream3->PAR = (uint32_t)&SPI1->DR;

  // enable DMA requests for SPI1
  SPI1->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

  // set interrupt priority
  NVIC->IPR[DMA2_Stream0_IRQn] = (0b0011 << 4);
  // enable DMA2 Stream0 interrupts
  NVIC->ISER[DMA2_Stream0_IRQn >> 5] = (1 << (DMA2_Stream0_IRQn & 0x1F));
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

void spi_dma_transfer(uint8_t* tx, uint8_t* rx, uint16_t len) {
  // ensure streams are disabled for configuration
  DMA2_Stream0->CR &= ~DMA_SxCR_EN;
  DMA2_Stream3->CR &= ~DMA_SxCR_EN;
  while ((DMA2_Stream0->CR & DMA_SxCR_EN) || (DMA2_Stream3->CR & DMA_SxCR_EN))
    ;

  // set memory addresses
  DMA2_Stream0->M0AR = (uint32_t)rx;
  DMA2_Stream3->M0AR = (uint32_t)tx;

  // set transaction length
  DMA2_Stream0->NDTR = len;
  DMA2_Stream3->NDTR = len;

  // clear all interrupt flags (write all 1s)
  // this technically writes over reserved regions, but whatever
  DMA2->LIFCR = UINT32_MAX;
  DMA2->HIFCR = UINT32_MAX;

  // assert chip select (active low)
  GPIOE->BSRR = GPIO_BSRR_BR3;

  // enable the DMA streams
  DMA2_Stream0->CR |= DMA_SxCR_EN;
  DMA2_Stream3->CR |= DMA_SxCR_EN;
}

[[gnu::weak]]
void spi_dma_complete() {
  // do nothing by default
  return;
}
