#ifndef SPI_H
#define SPI_H

#include <stdint.h>

// Configure SPI1 on PA[5..7]
void spi_init();

// Read data from `addr` using SPI1 in blocking mode
uint8_t spi_read(uint8_t addr);

// Write `data` to `addr` using SPI1 in blocking mode
void spi_write(uint8_t addr, uint8_t data);

#endif
