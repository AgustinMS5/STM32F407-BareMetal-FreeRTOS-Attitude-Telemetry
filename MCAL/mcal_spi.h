/**
 * @file    mcal_spi.h
 * @brief   Low-level MCAL interface for SPI1 peripheral (Direct CMSIS register access).
 * @author  Agustín Muñoz Santucho
 */

#ifndef MCAL_SPI_H_
#define MCAL_SPI_H_

#include <stdint.h>
#include "stm32f407xx.h"

void SPI1_Init(void);
void SPI1_CS_Low(void);
void SPI1_CS_High(void);
uint8_t SPI1_Transfer(uint8_t data);

#endif /* MCAL_SPI_H_ */
