/**
 * @file    main.c
 * @brief   System entry point, hardware self-test (Fail-Safe), and FreeRTOS scheduler start.
 * @author  Agustín Muñoz Santucho
 */

#include "stm32f407xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_leds.h"
#include "lis3dsh.h"
#include "app_tasks.h"

int main(void) {
    /* Configure NVIC for 4 bits of preemption priority (0 bits subpriority, required by FreeRTOS) */
    NVIC_SetPriorityGrouping(0x03U);

    LedsInit();

    /* Boot self-test: halt in fail-safe state (Red LED) if SPI bus or sensor ID check fails */
    if (!LIS3DSH_Init()) {
        BSP_LED_On(LED_RED);
        while (1);
    }

    App_Init();

    /* Hand over execution control to the FreeRTOS kernel */
    vTaskStartScheduler();

    while (1) {
        /* Should never reach here */
    }
}
