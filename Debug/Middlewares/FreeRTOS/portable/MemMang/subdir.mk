################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/FreeRTOS/portable/MemMang/heap_4.c 

OBJS += \
./Middlewares/FreeRTOS/portable/MemMang/heap_4.o 

C_DEPS += \
./Middlewares/FreeRTOS/portable/MemMang/heap_4.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/FreeRTOS/portable/MemMang/%.o Middlewares/FreeRTOS/portable/MemMang/%.su Middlewares/FreeRTOS/portable/MemMang/%.cyclo: ../Middlewares/FreeRTOS/portable/MemMang/%.c Middlewares/FreeRTOS/portable/MemMang/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F407xx -DSTM32 -DSTM32F4 -DSTM32F407VGTx -c -I../Core/Inc -I../App -I../BSP -I../MCAL -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Middlewares/FreeRTOS/include -I../Middlewares/FreeRTOS/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-FreeRTOS-2f-portable-2f-MemMang

clean-Middlewares-2f-FreeRTOS-2f-portable-2f-MemMang:
	-$(RM) ./Middlewares/FreeRTOS/portable/MemMang/heap_4.cyclo ./Middlewares/FreeRTOS/portable/MemMang/heap_4.d ./Middlewares/FreeRTOS/portable/MemMang/heap_4.o ./Middlewares/FreeRTOS/portable/MemMang/heap_4.su

.PHONY: clean-Middlewares-2f-FreeRTOS-2f-portable-2f-MemMang

