/**
 * @file    bsp_leds.h
 * @brief   Board Support Package (BSP) for TIM4 hardware PWM LED actuation.
 * @author  Agustín Muñoz Santucho
 */

#ifndef BSP_LEDS_H_
#define BSP_LEDS_H_

#include <stdint.h>
#include "stm32f407xx.h"

typedef enum {
    LED_GREEN  = 12, /* PD12 -> TIM4_CH1 (-X Axis) */
    LED_ORANGE = 13, /* PD13 -> TIM4_CH2 (+Y Axis) */
    LED_RED    = 14, /* PD14 -> TIM4_CH3 (+X Axis) */
    LED_BLUE   = 15  /* PD15 -> TIM4_CH4 (-Y Axis) */
} BSP_Led_t;

void LedsInit(void);
void BSP_LED_SetBrightness(BSP_Led_t led, uint16_t duty_permille);
void BSP_LED_On(BSP_Led_t led);
void BSP_LED_Off(BSP_Led_t led);
void BSP_LED_OffAll(void);

#endif /* BSP_LEDS_H_ */
