################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../stubs/cpp_stubs.cpp 

C_SRCS += \
../stubs/newlibc_stubs.c 

OBJS += \
./stubs/cpp_stubs.o \
./stubs/newlibc_stubs.o 

C_DEPS += \
./stubs/newlibc_stubs.d 

CPP_DEPS += \
./stubs/cpp_stubs.d 


# Each subdirectory must supply rules for building sources it contributes
stubs/%.o: ../stubs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-none-eabi-g++ -DUSE_STDPERIPH_DRIVER -DSTM32F10X_CL -I../Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x -I../Libraries/CMSIS/CM3/CoreSupport -I../Libraries/STM32F10x_StdPeriph_Driver/inc -I../src -I../libxsvf -I../include -I../. -I../Utilities -Os -fdata-sections -ffunction-sections -Wall -c -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -mlong-calls -std=gnu++0x -fno-rtti -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

stubs/%.o: ../stubs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -DUSE_STDPERIPH_DRIVER -DSTM32F10X_CL -I../Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x -I../Libraries/CMSIS/CM3/CoreSupport -I../Libraries/STM32F10x_StdPeriph_Driver/inc -I../src -I../libxsvf -I../include -I../. -I../Utilities -Os -fdata-sections -ffunction-sections -Wall -c -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -mlong-calls -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


