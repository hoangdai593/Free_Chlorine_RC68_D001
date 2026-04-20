################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_utils/Src/crc.c 

OBJS += \
./User_utils/Src/crc.o 

C_DEPS += \
./User_utils/Src/crc.d 


# Each subdirectory must supply rules for building sources it contributes
User_utils/Src/%.o User_utils/Src/%.su User_utils/Src/%.cyclo: ../User_utils/Src/%.c User_utils/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils/Inc" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app/Inc" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/Inc" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_utils-2f-Src

clean-User_utils-2f-Src:
	-$(RM) ./User_utils/Src/crc.cyclo ./User_utils/Src/crc.d ./User_utils/Src/crc.o ./User_utils/Src/crc.su

.PHONY: clean-User_utils-2f-Src

