################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_bsp/glcd-master/controllers/NT75451.c \
../User_bsp/glcd-master/controllers/PCD8544.c \
../User_bsp/glcd-master/controllers/ST7565R.c 

OBJS += \
./User_bsp/glcd-master/controllers/NT75451.o \
./User_bsp/glcd-master/controllers/PCD8544.o \
./User_bsp/glcd-master/controllers/ST7565R.o 

C_DEPS += \
./User_bsp/glcd-master/controllers/NT75451.d \
./User_bsp/glcd-master/controllers/PCD8544.d \
./User_bsp/glcd-master/controllers/ST7565R.d 


# Each subdirectory must supply rules for building sources it contributes
User_bsp/glcd-master/controllers/%.o User_bsp/glcd-master/controllers/%.su User_bsp/glcd-master/controllers/%.cyclo: ../User_bsp/glcd-master/controllers/%.c User_bsp/glcd-master/controllers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/event_driven" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/controllers" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/devices" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/fonts" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_bsp-2f-glcd-2d-master-2f-controllers

clean-User_bsp-2f-glcd-2d-master-2f-controllers:
	-$(RM) ./User_bsp/glcd-master/controllers/NT75451.cyclo ./User_bsp/glcd-master/controllers/NT75451.d ./User_bsp/glcd-master/controllers/NT75451.o ./User_bsp/glcd-master/controllers/NT75451.su ./User_bsp/glcd-master/controllers/PCD8544.cyclo ./User_bsp/glcd-master/controllers/PCD8544.d ./User_bsp/glcd-master/controllers/PCD8544.o ./User_bsp/glcd-master/controllers/PCD8544.su ./User_bsp/glcd-master/controllers/ST7565R.cyclo ./User_bsp/glcd-master/controllers/ST7565R.d ./User_bsp/glcd-master/controllers/ST7565R.o ./User_bsp/glcd-master/controllers/ST7565R.su

.PHONY: clean-User_bsp-2f-glcd-2d-master-2f-controllers

