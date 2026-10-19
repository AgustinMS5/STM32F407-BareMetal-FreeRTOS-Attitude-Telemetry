/**
 * @file    bsp_leds.c
 * @brief   Zero-CPU hardware PWM generation (1 kHz) using TIM4 channels on AF2.
 * @author  Agustín Muñoz Santucho
 */

#include "bsp_leds.h"

void LedsInit(void) {
    /* Enable clocks for GPIOD (AHB1) and TIM4 (APB1) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* Configure PD12..PD15 in Alternate Function mode (10b) */
    GPIOD->MODER   &= ~((3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2)));
    GPIOD->MODER   |=  ((2U << (12 * 2)) | (2U << (13 * 2)) | (2U << (14 * 2)) | (2U << (15 * 2)));
    GPIOD->OTYPER  &= ~((1U << 12) | (1U << 13) | (1U << 14) | (1U << 15));
    GPIOD->OSPEEDR &= ~((3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2)));
    GPIOD->PUPDR   &= ~((3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2)));

    /* Mux PD12..PD15 to TIM4_CH1..4 (AF2) via high register AFR[1] (pins 8..15) */
    GPIOD->AFR[1] &= ~((0x0FU << 16) | (0x0FU << 20) | (0x0FU << 24) | (0x0FU << 28));
    GPIOD->AFR[1] |=  ((2U << 16)    | (2U << 20)    | (2U << 24)    | (2U << 28));

    /* TIM4 timebase: 16 MHz / 16 = 1 MHz tick; ARR = 1000 steps -> 1 kHz PWM frequency */
    TIM4->PSC = 16U - 1U;
    TIM4->ARR = 1000U - 1U;

    /* Set CH1..CH4 to PWM Mode 1 (110b) with Output Compare Preload enabled (OCxPE) */
    TIM4->CCMR1 = (6U << 4) | (1U << 3) | (6U << 12) | (1U << 11);
    TIM4->CCMR2 = (6U << 4) | (1U << 3) | (6U << 12) | (1U << 11);

    /* Enable physical capture/compare output drivers for all 4 channels */
    TIM4->CCER |= (1U << 0) | (1U << 4) | (1U << 8) | (1U << 12);

    BSP_LED_OffAll();

    /* Enable Auto-Reload Preload, trigger shadow register update (UG), and start timer */
    TIM4->CR1 |= TIM_CR1_ARPE;
    TIM4->EGR |= TIM_EGR_UG;
    TIM4->CR1 |= TIM_CR1_CEN;
}

void BSP_LED_SetBrightness(BSP_Led_t led, uint16_t duty_permille) {
    /* Clamp duty cycle to 100% (1000 permille) */
    if (duty_permille > 1000U) duty_permille = 1000U;

    /* Update hardware Capture/Compare register (zero CPU intervention) */
    switch (led) {
        case LED_GREEN:  TIM4->CCR1 = duty_permille; break;
        case LED_ORANGE: TIM4->CCR2 = duty_permille; break;
        case LED_RED:    TIM4->CCR3 = duty_permille; break;
        case LED_BLUE:   TIM4->CCR4 = duty_permille; break;
        default: break;
    }
}

void BSP_LED_On(BSP_Led_t led)  { BSP_LED_SetBrightness(led, 1000U); }
void BSP_LED_Off(BSP_Led_t led) { BSP_LED_SetBrightness(led, 0U); }

void BSP_LED_OffAll(void) {
    TIM4->CCR1 = 0U;
    TIM4->CCR2 = 0U;
    TIM4->CCR3 = 0U;
    TIM4->CCR4 = 0U;
}
