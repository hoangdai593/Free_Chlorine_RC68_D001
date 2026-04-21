################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_bsp/LCD_NHD_C12864LZ/LCD_NHD.c 

OBJS += \
./User_bsp/LCD_NHD_C12864LZ/LCD_NHD.o 

C_DEPS += \
./User_bsp/LCD_NHD_C12864LZ/LCD_NHD.d 


# Each subdirectory must supply rules for building sources it contributes
User_bsp/LCD_NHD_C12864LZ/%.o User_bsp/LCD_NHD_C12864LZ/%.su User_bsp/LCD_NHD_C12864LZ/%.cyclo: ../User_bsp/LCD_NHD_C12864LZ/%.c User_bsp/LCD_NHD_C12864LZ/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/event_driven" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/LCD_NHD_C12864LZ" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_bsp-2f-LCD_NHD_C12864LZ

clean-User_bsp-2f-LCD_NHD_C12864LZ:
	-$(RM) ./User_bsp/LCD_NHD_C12864LZ/LCD_NHD.cyclo ./User_bsp/LCD_NHD_C12864LZ/LCD_NHD.d ./User_bsp/LCD_NHD_C12864LZ/LCD_NHD.o ./User_bsp/LCD_NHD_C12864LZ/LCD_NHD.su

.PHONY: clean-User_bsp-2f-LCD_NHD_C12864LZ

