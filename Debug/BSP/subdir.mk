################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/bsp_leds.c \
../BSP/lis3dsh.c 

OBJS += \
./BSP/bsp_leds.o \
./BSP/lis3dsh.o 

C_DEPS += \
./BSP/bsp_leds.d \
./BSP/lis3dsh.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/%.o BSP/%.su BSP/%.cyclo: ../BSP/%.c BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F407xx -DSTM32 -DSTM32F4 -DSTM32F407VGTx -c -I../Core/Inc -I../App -I../BSP -I../MCAL -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Middlewares/FreeRTOS/include -I../Middlewares/FreeRTOS/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSP

clean-BSP:
	-$(RM) ./BSP/bsp_leds.cyclo ./BSP/bsp_leds.d ./BSP/bsp_leds.o ./BSP/bsp_leds.su ./BSP/lis3dsh.cyclo ./BSP/lis3dsh.d ./BSP/lis3dsh.o ./BSP/lis3dsh.su

.PHONY: clean-BSP

