################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/fbyne/workspace/RTC/DS3232RTC/DS3232RTC.cpp 

CPP_DEPS += \
./DS3232RTC/DS3232RTC.cpp.d 

LINK_OBJ += \
./DS3232RTC/DS3232RTC.cpp.o 


# Each subdirectory must supply rules for building sources it contributes
DS3232RTC/DS3232RTC.cpp.o: /home/fbyne/workspace/RTC/DS3232RTC/DS3232RTC.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-g++" -c -g -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega328p -DF_CPU=8000000L -DARDUINO=10601 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR     -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/cores/arduino" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/variants/standard" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/SoftwareSerial" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/Wire" -I"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/arduino/avr/libraries/Wire/utility" -I"/home/fbyne/workspace/RTC/JeeLabs_RTC_libraries/Time" -I"/home/fbyne/workspace/RTC/DS3232RTC" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"   -Wall
	@echo 'Finished building: $<'
	@echo ' '


