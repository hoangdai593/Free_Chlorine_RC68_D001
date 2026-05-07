################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_bsp/rs485/bsp_rs485.c 

OBJS += \
./User_bsp/rs485/bsp_rs485.o 

C_DEPS += \
./User_bsp/rs485/bsp_rs485.d 


# Each subdirectory must supply rules for building sources it contributes
User_bsp/rs485/%.o User_bsp/rs485/%.su User_bsp/rs485/%.cyclo: ../User_bsp/rs485/%.c User_bsp/rs485/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/event_driven" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/controllers" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/devices" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/fonts" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/uart" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/rs485" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/timer" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/modbus" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_bsp-2f-rs485

clean-User_bsp-2f-rs485:
	-$(RM) ./User_bsp/rs485/bsp_rs485.cyclo ./User_bsp/rs485/bsp_rs485.d ./User_bsp/rs485/bsp_rs485.o ./User_bsp/rs485/bsp_rs485.su

.PHONY: clean-User_bsp-2f-rs485

