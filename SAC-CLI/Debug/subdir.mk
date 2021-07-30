################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../conexiones-cliente.c \
../sac-cli-checkpoint.c 

OBJS += \
./conexiones-cliente.o \
./sac-cli-checkpoint.o 

C_DEPS += \
./conexiones-cliente.d \
./sac-cli-checkpoint.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2019-2c-NoCurseCreatividad/herramientas" -O0 -g3 -Wall -c -fmessage-length=0 -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


