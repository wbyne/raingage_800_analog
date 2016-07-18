################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/Wire/Wire.cpp 

CPP_DEPS += \
./Libraries/Wire/Wire.cpp.d 

LINK_OBJ += \
./Libraries/Wire/Wire.cpp.o 


# Each subdirectory must supply rules for building sources it contributes
Libraries/Wire/Wire.cpp.o: /home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/Wire/Wire.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-g++" -c -g -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega328p -DF_CPU=8000000L -DARDUINO=10601 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR     -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/cores/arduino" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/variants/standard" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/SoftwareSerial" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/Wire" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/Wire/utility" -I"/home/fbyne/workspace/RTC/JeeLabs_RTC_libraries/Time" -I"/home/fbyne/workspace/RTC/DS3232RTC" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"   -Wall
	@echo 'Finished building: $<'
	@echo ' '


