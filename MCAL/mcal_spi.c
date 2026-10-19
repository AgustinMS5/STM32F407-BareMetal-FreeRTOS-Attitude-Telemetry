/**
 * @file    mcal_spi.c
 * @brief   Bare-metal SPI1 Master driver configured in Mode 3 (CPOL=1, CPHA=1).
 *          Pinout: PA5 (SCK), PA6 (MISO), PA7 (MOSI) on AF5 | PE3 (Manual Chip Select).
 * @author  Agustín Muñoz Santucho
 */

#include "mcal_spi.h"

void SPI1_Init(void) {
    /* Enable peripheral clocks for SPI1 (APB2) and GPIOA/GPIOE ports (AHB1) */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOEEN);

    /* Configure PE3 (CS) as high-speed Push-Pull output with no pull resistors */
    GPIOE->MODER   &= ~(3U << (3 * 2));
    GPIOE->MODER   |=  (1U << (3 * 2));
    GPIOE->OTYPER  &= ~(1U << 3);
    GPIOE->OSPEEDR |=  (3U << (3 * 2));
    GPIOE->PUPDR   &= ~(3U << (3 * 2));

    /* Drive CS high (idle state) prior to enabling the SPI bus */
    GPIOE->BSRR = (1U << 3);

    /* Configure PA5, PA6, PA7 in Alternate Function mode (10b) with high slew rate */
    GPIOA->MODER   &= ~((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA->MODER   |=  ((2U << (5 * 2)) | (2U << (6 * 2)) | (2U << (7 * 2)));
    GPIOA->OSPEEDR |=  ((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA->PUPDR   &= ~((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));

    /* Route PA5, PA6, PA7 to AF5 (SPI1) in the low alternate function register (AFRL) */
    GPIOA->AFR[0]  &= ~((0x0FU << (5 * 4)) | (0x0FU << (6 * 4)) | (0x0FU << (7 * 4)));
    GPIOA->AFR[0]  |=  ((5U << (5 * 4)) | (5U << (6 * 4)) | (5U << (7 * 4)));

    /* Ensure SPI1 is disabled before modifying CR1 configuration bits (RM0090) */
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1  = 0U;

    /* Master Mode | fPCLK/16 (1 MHz) | Mode 3 | Software NSS (prevents MODF fault) */
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= (3U << 3);
    SPI1->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);
    SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI);

    /* Enable SPI1 peripheral */
    SPI1->CR1 |= SPI_CR1_SPE;
}

void SPI1_CS_Low(void) {
    /* Atomic reset via upper half of BSRR: pulls PE3 low (active) */
    GPIOE->BSRR = (1U << (3 + 16));
}

void SPI1_CS_High(void) {
    /* Atomic set via lower half of BSRR: pulls PE3 high (idle) */
    GPIOE->BSRR = (1U << 3);
}

uint8_t SPI1_Transfer(uint8_t data) {
    /* Wait until Transmit Buffer is empty (TXE = 1) */
    while (!(SPI1->SR & SPI_SR_TXE));

    /* Write to Data Register to initiate 8 clock cycles on SCK */
    SPI1->DR = data;

    /* Wait until shift register completes reception (RXNE = 1) */
    while (!(SPI1->SR & SPI_SR_RXNE));

    /* Read received byte (reading DR automatically clears the RXNE hardware flag) */
    return (uint8_t)(SPI1->DR);
}


