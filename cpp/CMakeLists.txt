cmake_minimum_required(VERSION 3.10)

# Option to build for AVR or native
option(BUILD_FOR_AVR "Build for AVR microcontroller" ON)

if(BUILD_FOR_AVR)
    # Set toolchain before project() call
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR avr)

    # Set AVR toolchain paths
    set(AVR_TOOLCHAIN_PATH "/Users/alexpoulin/local/avr")
    set(CMAKE_C_COMPILER "${AVR_TOOLCHAIN_PATH}/bin/avr-gcc")
    set(CMAKE_CXX_COMPILER "${AVR_TOOLCHAIN_PATH}/bin/avr-g++")
    set(CMAKE_ASM_COMPILER "${AVR_TOOLCHAIN_PATH}/bin/avr-gcc")
    set(CMAKE_AR "${AVR_TOOLCHAIN_PATH}/bin/avr-ar")
    set(CMAKE_RANLIB "${AVR_TOOLCHAIN_PATH}/bin/avr-ranlib")
    set(CMAKE_OBJCOPY "${AVR_TOOLCHAIN_PATH}/bin/avr-objcopy")
    set(CMAKE_OBJDUMP "${AVR_TOOLCHAIN_PATH}/bin/avr-objdump")
    set(CMAKE_SIZE "${AVR_TOOLCHAIN_PATH}/bin/avr-size")

    # Skip compiler tests for cross-compilation
    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)
    set(CMAKE_ASM_COMPILER_WORKS 1)

    # Set target properties for AVR
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

project(needliest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory
include_directories(include)

if(BUILD_FOR_AVR)
    # Arduino Uno (ATmega328P) settings
    set(MCU "atmega328p")
    set(F_CPU "16000000UL")

    # AVR specific compiler flags
    set(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -fno-lto")
    set(CMAKE_CXX_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -std=c++17 -fno-lto")
    set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU} -fno-lto")
    
    message(STATUS "Building for AVR (${MCU}) at ${F_CPU}")
else()
    # Native build flags
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")
    message(STATUS "Building for native platform")
endif()

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(my_program ${SOURCES})

if(BUILD_FOR_AVR)
    # Custom target to generate hex file
    add_custom_target(hex
        COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom $<TARGET_FILE:my_program> ${CMAKE_BINARY_DIR}/my_program.hex
        DEPENDS my_program
        COMMENT "Generating hex file for AVR"
    )

    # Custom target to show program size
    add_custom_target(size
        COMMAND ${CMAKE_SIZE} -B $<TARGET_FILE:my_program>
        DEPENDS my_program
        COMMENT "Showing program size"
    )

    # Custom target to generate assembly listing
    add_custom_target(listing
        COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:my_program> > ${CMAKE_BINARY_DIR}/my_program.lst
        DEPENDS my_program
        COMMENT "Generating assembly listing"
    )

    # Custom target for uploading to Arduino (if avrdude is available)
    if(EXISTS "${AVR_TOOLCHAIN_PATH}/bin/avrdude")
        add_custom_target(upload
            COMMAND ${AVR_TOOLCHAIN_PATH}/bin/avrdude -c arduino -p ${MCU} -P /dev/cu.usbmodem* -b 115200 -U flash:w:${CMAKE_BINARY_DIR}/my_program.hex:i
            DEPENDS hex
            COMMENT "Uploading to Arduino Uno"
        )
    endif()
endif()