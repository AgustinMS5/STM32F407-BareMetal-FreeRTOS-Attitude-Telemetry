################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c 

OBJS += \
./Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.o 

C_DEPS += \
./Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/%.o Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/%.su Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/%.cyclo: ../Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/%.c Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F407xx -DSTM32 -DSTM32F4 -DSTM32F407VGTx -c -I../Core/Inc -I../App -I../BSP -I../MCAL -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Middlewares/FreeRTOS/include -I../Middlewares/FreeRTOS/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM4F

clean-Middlewares-2f-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM4F:
	-$(RM) ./Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.cyclo ./Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.d ./Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.o ./Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.su

.PHONY: clean-Middlewares-2f-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM4F

