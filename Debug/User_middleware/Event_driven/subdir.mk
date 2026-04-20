################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_middleware/Event_driven/event_driven.c 

OBJS += \
./User_middleware/Event_driven/event_driven.o 

C_DEPS += \
./User_middleware/Event_driven/event_driven.d 


# Each subdirectory must supply rules for building sources it contributes
User_middleware/Event_driven/%.o User_middleware/Event_driven/%.su User_middleware/Event_driven/%.cyclo: ../User_middleware/Event_driven/%.c User_middleware/Event_driven/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/Event_driven" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_middleware-2f-Event_driven

clean-User_middleware-2f-Event_driven:
	-$(RM) ./User_middleware/Event_driven/event_driven.cyclo ./User_middleware/Event_driven/event_driven.d ./User_middleware/Event_driven/event_driven.o ./User_middleware/Event_driven/event_driven.su

.PHONY: clean-User_middleware-2f-Event_driven

