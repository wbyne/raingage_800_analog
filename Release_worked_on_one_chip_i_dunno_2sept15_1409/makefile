################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include arduino/core/subdir.mk
-include Time/examples/TimeTeensy3/subdir.mk
-include Time/examples/TimeSerialDateStrings/subdir.mk
-include Time/examples/TimeSerial/subdir.mk
-include Time/examples/TimeRTCSet/subdir.mk
-include Time/examples/TimeNTP/subdir.mk
-include Time/examples/TimeGPS/subdir.mk
-include Time/examples/TimeArduinoDue/subdir.mk
-include Time/subdir.mk
-include Libraries/Wire/utility/subdir.mk
-include Libraries/Wire/subdir.mk
-include Libraries/SoftwareSerial/subdir.mk
-include DS3232RTC/examples/tiny3232_KnockBang/subdir.mk
-include DS3232RTC/examples/TimeRTC/subdir.mk
-include DS3232RTC/examples/SetSerial/subdir.mk
-include DS3232RTC/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
AR += \
arduino.ar \

ELF += \
raingage_800_analog.elf \

EEP += \
raingage_800_analog.eep \

FLASH_IMAGE += \
raingage_800_analog.hex \


# All Target
all: raingage_800_analog

# Tool invocations
raingage_800_analog: $(FLASH_IMAGE)
	@echo 'Building target: $@'
	@echo 'Printing size:'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-size" -A "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/raingage_800_analog.elf"
	@echo 'Finished building target: $@'
	@echo ' '

arduino.ar: $(AR_OBJ)
	@echo 'Starting archiver'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-ar" rcs  "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/arduino.ar"   $(AR_OBJ)
	@echo 'Finished building: $@'
	@echo ' '

raingage_800_analog.elf: $(LINK_OBJ) $(AR)
	@echo 'Starting combiner'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-gcc" -Os -Wl,--gc-sections -mmcu=atmega328p  -o "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/raingage_800_analog.elf"   $(LINK_OBJ) $(AR)  "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/arduino.ar" "-L/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release" -lm
	@echo 'Finished building: $@'
	@echo ' '

raingage_800_analog.eep: $(ELF)
	@echo 'Create eeprom image'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-objcopy" -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0  "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/raingage_800_analog.elf" "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/raingage_800_analog.eep"
	@echo 'Finished building: $@'
	@echo ' '

raingage_800_analog.hex: $(EEP)
	@echo 'Create Flash image (ihex format)'
	"/home/fbyne/workspace/arduino_old_versions/arduino-1.6.1/hardware/tools/avr/bin/avr-objcopy" -O ihex -R .eeprom  "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/raingage_800_analog.elf" "/home/fbyne/workspace/eclipseArduino/workspace/raingage_800_analog/Release/raingage_800_analog.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C_DEPS)$(AR)$(FLASH_IMAGE)$(EEP)$(CXX_DEPS)$(S_UPPER_DEPS)$(ELF)$(C++_DEPS)$(ASM_DEPS)$(CC_DEPS)$(INO_DEPS)$(AR_OBJ)$(CPP_DEPS)$(LINK_OBJ)$(C_UPPER_DEPS)$(SIZEDUMMY) raingage_800_analog
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
