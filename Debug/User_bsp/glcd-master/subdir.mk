################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User_bsp/glcd-master/glcd.c \
../User_bsp/glcd-master/graphics.c \
../User_bsp/glcd-master/graphs.c \
../User_bsp/glcd-master/text.c \
../User_bsp/glcd-master/text_tiny.c \
../User_bsp/glcd-master/unit_tests.c 

OBJS += \
./User_bsp/glcd-master/glcd.o \
./User_bsp/glcd-master/graphics.o \
./User_bsp/glcd-master/graphs.o \
./User_bsp/glcd-master/text.o \
./User_bsp/glcd-master/text_tiny.o \
./User_bsp/glcd-master/unit_tests.o 

C_DEPS += \
./User_bsp/glcd-master/glcd.d \
./User_bsp/glcd-master/graphics.d \
./User_bsp/glcd-master/graphs.d \
./User_bsp/glcd-master/text.d \
./User_bsp/glcd-master/text_tiny.d \
./User_bsp/glcd-master/unit_tests.d 


# Each subdirectory must supply rules for building sources it contributes
User_bsp/glcd-master/%.o User_bsp/glcd-master/%.su User_bsp/glcd-master/%.cyclo: ../User_bsp/glcd-master/%.c User_bsp/glcd-master/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L433xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_app" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_utils" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_middleware/event_driven" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/controllers" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/devices" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master/fonts" -I"D:/STM32_workspace/Free_Chlorine_RC68_D001/User_bsp/glcd-master" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User_bsp-2f-glcd-2d-master

clean-User_bsp-2f-glcd-2d-master:
	-$(RM) ./User_bsp/glcd-master/glcd.cyclo ./User_bsp/glcd-master/glcd.d ./User_bsp/glcd-master/glcd.o ./User_bsp/glcd-master/glcd.su ./User_bsp/glcd-master/graphics.cyclo ./User_bsp/glcd-master/graphics.d ./User_bsp/glcd-master/graphics.o ./User_bsp/glcd-master/graphics.su ./User_bsp/glcd-master/graphs.cyclo ./User_bsp/glcd-master/graphs.d ./User_bsp/glcd-master/graphs.o ./User_bsp/glcd-master/graphs.su ./User_bsp/glcd-master/text.cyclo ./User_bsp/glcd-master/text.d ./User_bsp/glcd-master/text.o ./User_bsp/glcd-master/text.su ./User_bsp/glcd-master/text_tiny.cyclo ./User_bsp/glcd-master/text_tiny.d ./User_bsp/glcd-master/text_tiny.o ./User_bsp/glcd-master/text_tiny.su ./User_bsp/glcd-master/unit_tests.cyclo ./User_bsp/glcd-master/unit_tests.d ./User_bsp/glcd-master/unit_tests.o ./User_bsp/glcd-master/unit_tests.su

.PHONY: clean-User_bsp-2f-glcd-2d-master

