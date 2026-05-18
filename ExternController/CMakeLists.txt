cmake_minimum_required(VERSION 3.10)

project(pico_serial_controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(serial_controller
    main.cpp
)
