cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set Pico SDK path
set(PICO_SDK_PATH "/home/victor/Pico/pico-sdk")

# VS Code Pico extension support
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# Board type
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Import Pico SDK
include(pico_sdk_import.cmake)

# Project name
project(GripperCode C CXX ASM)

# Initialize SDK
pico_sdk_init()

# Create executable from your source file
add_executable(GripperCode
    GripperCode.cpp
)

# Program info
pico_set_program_name(GripperCode "GripperCode")
pico_set_program_version(GripperCode "0.1")

# USB/UART settings
pico_enable_stdio_uart(GripperCode 0)
pico_enable_stdio_usb(GripperCode 1)

# Libraries
target_link_libraries(GripperCode
    pico_stdlib
    hardware_adc
    hardware_pwm
    pico_multicore
)

# Include directories
target_include_directories(GripperCode PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Generate UF2/ELF/etc
pico_add_extra_outputs(GripperCode)
