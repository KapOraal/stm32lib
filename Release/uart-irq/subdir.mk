################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../uart-irq/uart-irq.c 

OBJS += \
./uart-irq/uart-irq.o 

C_DEPS += \
./uart-irq/uart-irq.d 


# Each subdirectory must supply rules for building sources it contributes
uart-irq/%.o: ../uart-irq/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD_VL -I../Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x -I../Libraries/CMSIS/CM3/CoreSupport -I../Libraries/STM32F10x_StdPeriph_Driver/inc -I../src -I../libxsvf -I../include -I../. -I../Utilities -Os -fdata-sections -ffunction-sections -Wall -c -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -mlong-calls -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


