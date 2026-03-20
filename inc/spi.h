#ifndef SPI_H
#define SPI_H

#include <stdint.h>

// Configure SPI1 on PA[5..7]
void spi_init();

// Read data from `addr` using SPI1 in blocking mode
uint8_t spi_read(uint8_t addr);

// Write `data` to `addr` using SPI1 in blocking mode
void spi_write(uint8_t addr, uint8_t data);

// Configure SPI1 for DMA
void spi_dma_init();

// Start a DMA transfer for the SPI1.
// `tx` and `rx` shall be of the same length equal to `len`.
void spi_dma_transfer(uint8_t const* tx, uint8_t* rx, uint16_t len);

// User callback for the DMA transfer completion
[[ gnu::weak ]]
void spi_dma_complete();

#endif
