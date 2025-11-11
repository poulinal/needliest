# AVR Toolchain file for CMake
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/avr-toolchain.cmake ..

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

# Set AVR toolchain paths
set(AVR_TOOLCHAIN_PATH "/Users/alexpoulin/local/avr" CACHE PATH "Path to AVR toolchain")

# Set compilers and tools
set(CMAKE_C_COMPILER "${AVR_TOOLCHAIN_PATH}/bin/avr-gcc")
set(CMAKE_CXX_COMPILER "${AVR_TOOLCHAIN_PATH}/bin/avr-g++")
set(CMAKE_ASM_COMPILER "${AVR_TOOLCHAIN_PATH}/bin/avr-gcc")
set(CMAKE_AR "${AVR_TOOLCHAIN_PATH}/bin/avr-ar")
set(CMAKE_RANLIB "${AVR_TOOLCHAIN_PATH}/bin/avr-ranlib")
set(CMAKE_OBJCOPY "${AVR_TOOLCHAIN_PATH}/bin/avr-objcopy")
set(CMAKE_OBJDUMP "${AVR_TOOLCHAIN_PATH}/bin/avr-objdump")
set(CMAKE_SIZE "${AVR_TOOLCHAIN_PATH}/bin/avr-size")

# Set find root path
set(CMAKE_FIND_ROOT_PATH "${AVR_TOOLCHAIN_PATH}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Arduino Uno (ATmega328P) default settings
set(MCU "atmega328p" CACHE STRING "AVR microcontroller")
set(F_CPU "16000000UL" CACHE STRING "CPU frequency")

# AVR specific compiler flags
set(CMAKE_C_FLAGS_INIT "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -fno-lto")
set(CMAKE_CXX_FLAGS_INIT "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -std=c++17 -fno-lto")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mmcu=${MCU} -fno-lto")

# Set file extensions
set(CMAKE_EXECUTABLE_SUFFIX ".elf")