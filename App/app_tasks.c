/**
 * @file    app_tasks.c
 * @brief   Real-time pipeline: 50 Hz jitter-free sampling, fixed-point IIR filtering,
 *          proportional PWM actuation, IWDG supervision, and SRAM telemetry exposure.
 * @author  Agustín Muñoz Santucho
 */

#include "app_tasks.h"
#include "stm32f407xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lis3dsh.h"
#include "bsp_leds.h"

#define TILT_DEADZONE   1200  /* Deadband threshold to reject flat-surface sensor noise */
#define TILT_MAX_RANGE  14000 /* Saturation threshold (~1G) for 100% PWM duty cycle */

/* Global telemetry block exposed in SRAM for non-intrusive CoreSight AHB-AP inspection */
typedef struct {
    int16_t  raw_x;
    int16_t  raw_y;
    int16_t  raw_z;
    int16_t  filt_x;
    int16_t  filt_y;
    int16_t  filt_z;
    uint16_t pwm_green;
    uint16_t pwm_orange;
    uint16_t pwm_red;
    uint16_t pwm_blue;
} Telemetry_t;

volatile Telemetry_t g_telemetry = {0};

static QueueHandle_t xQueueAccel = NULL;

/* Initialize Independent Watchdog (32 kHz LSI / 32 = 1 kHz -> 500 ms timeout) */
static void IWDG_Init(void) {
    IWDG->KR  = 0x5555U; /* Unlock PR and RLR registers */
    IWDG->PR  = 3U;      /* Prescaler /32 */
    IWDG->RLR = 500U;    /* 500 ms reload counter */
    IWDG->KR  = 0xAAAAU; /* Reload watchdog counter */
    IWDG->KR  = 0xCCCCU; /* Enable IWDG peripheral */
}

static void IWDG_Feed(void) {
    IWDG->KR = 0xAAAAU;  /* Refresh watchdog timer */
}

/* Linearly maps tilt magnitude to PWM duty cycle (0..1000 permille) */
static uint16_t Calculate_PWM_Duty(int16_t val) {
    if (val < 0) val = -val;
    if (val <= TILT_DEADZONE)  return 0U;
    if (val >= TILT_MAX_RANGE) return 1000U;
    return (uint16_t)(((uint32_t)(val - TILT_DEADZONE) * 1000U) / (TILT_MAX_RANGE - TILT_DEADZONE));
}

/* Producer Task (High Priority): Deterministic 50 Hz acquisition and fixed-point DSP */
static void vTaskSensor(void *pvParameters) {
    (void)pvParameters;
    LIS3DSH_Axes_t raw;
    LIS3DSH_Axes_t filt = {0, 0, 16000};

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(20); /* Strict 20 ms sampling period */

    for (;;) {
        LIS3DSH_ReadAxes(&raw);

        /* First-order fixed-point IIR Exponential Moving Average: y[n] = (3*y[n-1] + x[n]) >> 2 */
        filt.x = (int16_t)(((int32_t)filt.x * 3 + raw.x) >> 2);
        filt.y = (int16_t)(((int32_t)filt.y * 3 + raw.y) >> 2);
        filt.z = (int16_t)(((int32_t)filt.z * 3 + raw.z) >> 2);

        /* Update SRAM telemetry structure for live host visualization */
        g_telemetry.raw_x  = raw.x;
        g_telemetry.raw_y  = raw.y;
        g_telemetry.raw_z  = raw.z;
        g_telemetry.filt_x = filt.x;
        g_telemetry.filt_y = filt.y;
        g_telemetry.filt_z = filt.z;

        /* Enqueue filtered sample by copy (thread-safe IPC) */
        xQueueSend(xQueueAccel, &filt, 0U);

        /* Absolute delay guarantees jitter-free 50 Hz execution */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* Consumer Task (Normal Priority): Proportional PWM actuation and Watchdog refresh */
static void vTaskLEDs(void *pvParameters) {
    (void)pvParameters;
    LIS3DSH_Axes_t axes;

    for (;;) {
        /* Block indefinitely (0% CPU usage) until new sample arrives in queue */
        if (xQueueReceive(xQueueAccel, &axes, portMAX_DELAY) == pdPASS) {
            uint16_t green = 0, orange = 0, red = 0, blue = 0;

            if (axes.x > TILT_DEADZONE)       red    = Calculate_PWM_Duty(axes.x);
            else if (axes.x < -TILT_DEADZONE) green  = Calculate_PWM_Duty(axes.x);

            if (axes.y > TILT_DEADZONE)       orange = Calculate_PWM_Duty(axes.y);
            else if (axes.y < -TILT_DEADZONE) blue   = Calculate_PWM_Duty(axes.y);

            BSP_LED_SetBrightness(LED_GREEN,  green);
            BSP_LED_SetBrightness(LED_ORANGE, orange);
            BSP_LED_SetBrightness(LED_RED,    red);
            BSP_LED_SetBrightness(LED_BLUE,   blue);

            g_telemetry.pwm_green  = green;
            g_telemetry.pwm_orange = orange;
            g_telemetry.pwm_red    = red;
            g_telemetry.pwm_blue   = blue;

            /* Feed watchdog exclusively after full pipeline completion */
            IWDG_Feed();
        }
    }
}

void App_Init(void) {
    xQueueAccel = xQueueCreate(5, sizeof(LIS3DSH_Axes_t));
    if (xQueueAccel != NULL) {
        IWDG_Init();
        xTaskCreate(vTaskSensor, "SensorTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
        xTaskCreate(vTaskLEDs,   "LEDsTask",   configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    }
}
