cmake_minimum_required(VERSION 3.12)

# Include Pico SDK directly
include(/home/jack/pico/pico-sdk/pico_sdk_init.cmake)

project(rp2040-based-eeprom C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add main.c to the rp2040-based-eeprom build
add_executable(rp2040-based-eeprom
	main.c)

# Add the Pico libraries to the rp2040-based-eeprom build
target_link_libraries(rp2040-based-eeprom
	pico_stdlib
	hardware_i2c
	hardware_irq)

# Build the firmware using the Pico SDK
pico_add_extra_outputs(rp2040-based-eeprom)