/**
 * @file    lis3dsh.h
 * @brief   BSP driver for the onboard LIS3DSH 3-axis MEMS accelerometer.
 * @author  Agustín Muñoz Santucho
 */

#ifndef BSP_LIS3DSH_H_
#define BSP_LIS3DSH_H_

#include <stdint.h>
#include <stdbool.h>

/* LIS3DSH internal register map */
#define LIS3DSH_REG_WHO_AM_I    0x0F
#define LIS3DSH_REG_CTRL_REG4   0x20
#define LIS3DSH_REG_CTRL_REG6   0x25
#define LIS3DSH_REG_OUT_X_L     0x28
#define LIS3DSH_WHO_AM_I_VAL    0x3F

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} LIS3DSH_Axes_t;

bool LIS3DSH_Init(void);
void LIS3DSH_ReadAxes(LIS3DSH_Axes_t *axes);

#endif /* BSP_LIS3DSH_H_ */
