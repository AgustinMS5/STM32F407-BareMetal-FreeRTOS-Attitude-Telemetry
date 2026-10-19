/**
 * @file    lis3dsh.c
 * @brief   LIS3DSH SPI register protocol and 6-byte auto-increment burst acquisition.
 * @author  Agustín Muñoz Santucho
 */

#include "lis3dsh.h"
#include "mcal_spi.h"

/* Single-byte register write (MSB = 0) */
static void LIS3DSH_WriteReg(uint8_t reg, uint8_t val) {
    SPI1_CS_Low();
    SPI1_Transfer(reg & 0x7FU);
    SPI1_Transfer(val);
    SPI1_CS_High();
}

/* Single-byte register read (MSB = 1) + dummy byte clock generation */
static uint8_t LIS3DSH_ReadReg(uint8_t reg) {
    uint8_t val;
    SPI1_CS_Low();
    SPI1_Transfer(reg | 0x80U);
    val = SPI1_Transfer(0x00U);
    SPI1_CS_High();
    return val;
}

bool LIS3DSH_Init(void) {
    SPI1_Init();

    /* Verify silicon device ID over SPI before configuring control registers */
    if (LIS3DSH_ReadReg(LIS3DSH_REG_WHO_AM_I) != LIS3DSH_WHO_AM_I_VAL) {
        return false;
    }

    /* Enable register address auto-increment (ADD_INC) for multi-byte burst reads */
    LIS3DSH_WriteReg(LIS3DSH_REG_CTRL_REG6, 0x10U);

    /* Configure CTRL_REG4: 100 Hz ODR (0x60) | BDU enabled (0x08) | X/Y/Z enabled (0x07) */
    LIS3DSH_WriteReg(LIS3DSH_REG_CTRL_REG4, 0x6FU);

    return true;
}

void LIS3DSH_ReadAxes(LIS3DSH_Axes_t *axes) {
    uint8_t raw[6];

    /* Perform atomic 6-byte SPI burst read from OUT_X_L (0x28) to OUT_Z_H (0x2D) */
    SPI1_CS_Low();
    SPI1_Transfer(LIS3DSH_REG_OUT_X_L | 0x80U);
    for (int i = 0; i < 6; i++) {
        raw[i] = SPI1_Transfer(0x00U);
    }
    SPI1_CS_High();

    /* Reconstruct signed 16-bit two's complement values (Little-Endian) */
    axes->x = (int16_t)((raw[1] << 8) | raw[0]);
    axes->y = (int16_t)((raw[3] << 8) | raw[2]);
    axes->z = (int16_t)((raw[5] << 8) | raw[4]);
}
