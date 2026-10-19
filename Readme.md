# Bare-Metal STM32F407 3-Axis Inertial Attitude System, Hardware PWM & CoreSight 3D Telemetry

![Target](https://img.shields.io/badge/MCU-STM32F407VGT6%20(Cortex--M4F)-blue)
![Architecture](https://img.shields.io/badge/Architecture-Bare--Metal%20CMSIS%20(No%20HAL)-success)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS%20v10-orange)
![Instrumentation](https://img.shields.io/badge/Telemetry-ARM%20CoreSight%20AHB--AP-purple)

A deterministic, layered embedded firmware system engineered **from scratch without STM32 HAL/LL libraries** on the **STM32F407G-DISC1** (ARM Cortex-M4F). The system acquires 3-axis MEMS inertial data over a register-level SPI driver, filters mechanical noise via fixed-point DSP inside a FreeRTOS producer-consumer pipeline, drives 4-quadrant proportional hardware PWM indicators with 0% CPU load, and streams real-time telemetry to a 3D Digital Twin & DSP Oscilloscope over the ARM CoreSight debug bus without halting the CPU.

---


## Live Demonstration (Hardware + Real-Time 3D Digital Twin & DSP Scope)

[![STM32F407 Real-Time 3D Digital Twin & DSP Scope](https://img.youtube.com/vi/j0DCT61kyJc/maxresdefault.jpg)](https://www.youtube.com/watch?v=j0DCT61kyJc)

> 👆 **Click the image above to watch the hardware demonstration and real-time CoreSight telemetry on YouTube:** [https://youtu.be/j0DCT61kyJc](https://youtu.be/j0DCT61kyJc)

---

## Key Engineering Highlights

* **Zero-HAL Layered Architecture (`MCAL` / `BSP` / `App`):** Strict hardware decoupling following industrial embedded standards. All peripherals (`RCC`, `GPIO`, `SPI1`, `TIM4`, `IWDG`, `NVIC`) are configured via direct CMSIS register manipulation.
* **Register-Level SPI1 Driver & 6-Byte Burst Read:**
  * Configured in **SPI Mode 3 (`CPOL=1, CPHA=1`)** with Software Slave Management (`SSM=1, SSI=1`) to prevent Master Mode Fault (`MODF`) bus lockups.
  * Performs hardware identity verification (`WHO_AM_I == 0x3F`), enables **Block Data Update (`BDU`)** to prevent high/low byte tearing, and utilizes address auto-increment (`ADD_INC`) to execute atomic **6-byte SPI burst reads** (`X, Y, Z` axes in a single Chip Select transaction).
* **Deterministic FreeRTOS Pipeline & Fixed-Point IIR DSP:**
  * **Producer Task (`vTaskSensor`):** Executes at a strict, jitter-free **50 Hz (`20 ms`)** cadence using `vTaskDelayUntil()`. Implements a first-order **Exponential Moving Average (IIR) low-pass filter** in 32-bit fixed-point arithmetic using single-cycle bit shifts (`>> 2`), completely avoiding floating-point unit (FPU) overhead:
    $$y[n] = \frac{3 \cdot y[n-1] + x[n]}{4} \equiv \texttt{(filt * 3 + raw) >> 2}$$
  * **Consumer Task (`vTaskLEDs`):** Blocks indefinitely (`portMAX_DELAY`, **0% CPU load**) waiting on a thread-safe pass-by-copy FreeRTOS Queue (`xQueueAccel`).
* **Zero-CPU Hardware PWM Actuation (`TIM4`):**
  * Multiplexes pins `PD12–PD15` to `TIM4_CH1–CH4` (`AF2` via `GPIOD->AFR[1]`).
  * Configured with a `1 MHz` counter clock (`PSC = 15`) and `ARR = 999` (`1 kHz` PWM frequency), dynamically updating capture/compare registers (`CCR1–CCR4`) proportionally to the tilt angle outside a configurable deadzone.
* **Fault-Tolerant Silicon Watchdog (`IWDG`):**
  * Clocked by the independent `32 kHz` LSI hardware oscillator (`500 ms` timeout). Refreshed exclusively at the end of the consumer task to guarantee automated MCU recovery upon SPI bus stalls or task starvation.
* **Non-Intrusive ARM CoreSight AHB-AP Live Telemetry:**
  * Parses the linker-generated ELF memory map (`.map`) to locate the `volatile Telemetry_t g_telemetry` SRAM structure at runtime.
  * Reads 20-byte telemetry frames directly from SRAM through the Cortex-M4 AHB-AP debug port via OpenOCD + ST-LINK while the CPU runs at full speed (0 CPU cycle overhead), driving a 50 FPS 3D board model and live raw-vs-filtered DSP oscilloscope.

---

## System Architecture

```mermaid
flowchart TB
    subgraph HOST["💻 HOST PC (Linux Instrumentation)"]
        PY["<b>dashboard_3d.py</b><br/>ELF .map Parser + 50 FPS 3D Attitude Twin + Live DSP Scope"]
    end

    subgraph MCU["⚡ STM32F407VGT6 (ARM Cortex-M4F @ 16 MHz)"]
        SRAM[("<b>SRAM Telemetry Block</b><br/>volatile Telemetry_t g_telemetry")]

        subgraph APP["App Layer (FreeRTOS Kernel)"]
            PROD["<b>vTaskSensor (Producer)</b><br/>50 Hz vTaskDelayUntil<br/>Fixed-Point IIR Filter (>> 2)"]
            QUEUE[["<b>xQueueAccel</b><br/>Thread-Safe IPC (Pass-by-Copy)"]]
            CONS["<b>vTaskLEDs (Consumer)</b><br/>Deadzone + Duty Cycle Mapping"]
        end

        subgraph BSP["BSP Layer (Board Abstractions)"]
            MEMS["<b>lis3dsh.c</b><br/>WHO_AM_I (0x3F) | BDU | 6-Byte Burst Read"]
            LEDS["<b>bsp_leds.c</b><br/>4-Quadrant Proportional Brightness"]
            WDT["<b>IWDG Hardware</b><br/>32 kHz LSI | 500 ms Timeout"]
        end

        subgraph MCAL["MCAL Layer (Direct CMSIS Registers)"]
            SPI["<b>mcal_spi.c (SPI1 Mode 3)</b><br/>PA5/6/7 (AF5) | PE3 Atomic BSRR CS | SSM/SSI"]
            TIM["<b>TIM4 Hardware PWM</b><br/>PD12..15 (AF2 in AFR[1]) | 1 kHz | 0% CPU"]
        end
    end

    SENSOR(("<b>LIS3DSH MEMS</b><br/>3-Axis Sensor"))

    SENSOR <==>|"Physical SPI Bus"| SPI
    SPI ==> MEMS
    MEMS ==> PROD
    PROD ==>|"xQueueSend"| QUEUE
    QUEUE ==>|"xQueueReceive"| CONS
    PROD -.->|"Write Raw & Filtered"| SRAM
    CONS -.->|"Write PWM Duty"| SRAM
    CONS ==> LEDS
    CONS ==>|"IWDG_Feed (0xAAAA)"| WDT
    LEDS ==>|"CCR1..CCR4"| TIM
    SRAM ===>|"CoreSight AHB-AP (ST-LINK USB)<br/>0% CPU Overhead"| PY

    style HOST fill:#1e293b,stroke:#38bdf8,stroke-width:2px,color:#fff
    style MCU fill:#0f172a,stroke:#10b981,stroke-width:2px,color:#fff
    style APP fill:#1e1b4b,stroke:#818cf8,stroke-width:1px,color:#fff
    style BSP fill:#14532d,stroke:#4ade80,stroke-width:1px,color:#fff
    style MCAL fill:#3b0764,stroke:#c084fc,stroke-width:1px,color:#fff
```
