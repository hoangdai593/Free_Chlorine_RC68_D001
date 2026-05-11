################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_middleware/modbus/modbus_crc.c \
../User_middleware/modbus/modbus_master.c \
../User_middleware/modbus/modbus_rtu.c \
../User_middleware/modbus/modbus_slave.c 

OBJS += \
./User_middleware/modbus/modbus_crc.o \
./User_middleware/modbus/modbus_master.o \
./User_middleware/modbus/modbus_rtu.o \
./User_middleware/modbus/modbus_slave.o 

C_DEPS += \
./User_middleware/modbus/modbus_crc.d \
./User_middleware/modbus/modbus_master.d \
./User_middleware/modbus/modbus_rtu.d \
./User_middleware/modbus/modbus_slave.d 


# Each subdirectory must supply rules for building sources it contributes
User_middleware/modbus/%.o User_middleware/modbus/%.su User_middleware/modbus/%.cyclo: ../User_middleware/modbus/%.c User_middleware/modbus/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/event_driven" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/controllers" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/devices" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/fonts" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/uart" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/rs485" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/timer" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/modbus" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_middleware-2f-modbus

clean-User_middleware-2f-modbus:
	-$(RM) ./User_middleware/modbus/modbus_crc.cyclo ./User_middleware/modbus/modbus_crc.d ./User_middleware/modbus/modbus_crc.o ./User_middleware/modbus/modbus_crc.su ./User_middleware/modbus/modbus_master.cyclo ./User_middleware/modbus/modbus_master.d ./User_middleware/modbus/modbus_master.o ./User_middleware/modbus/modbus_master.su ./User_middleware/modbus/modbus_rtu.cyclo ./User_middleware/modbus/modbus_rtu.d ./User_middleware/modbus/modbus_rtu.o ./User_middleware/modbus/modbus_rtu.su ./User_middleware/modbus/modbus_slave.cyclo ./User_middleware/modbus/modbus_slave.d ./User_middleware/modbus/modbus_slave.o ./User_middleware/modbus/modbus_slave.su

.PHONY: clean-User_middleware-2f-modbus

