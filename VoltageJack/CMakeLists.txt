cmake_minimum_required(VERSION 3.31)
project(untitled)

set(CMAKE_CXX_STANDARD 20)


set(LABJACK_DIR "C:/Program Files (x86)/LabJack/Drivers")

# Include directory
include_directories("${LABJACK_DIR}/C_C++_LJM")

# Link directory
link_directories("${LABJACK_DIR}")

add_executable(untitled main.cpp
        VoltageJack.cpp)
# Link the library
target_link_libraries(untitled LabJackM)


