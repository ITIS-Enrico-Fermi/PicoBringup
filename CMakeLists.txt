cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
include(pico_flash_uf2.cmake)

project(picobringup C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(picobringup
  main.c
)

pico_enable_stdio_usb(picobringup 1)
pico_enable_stdio_uart(picobringup 1)
pico_add_extra_outputs(picobringup)

target_link_libraries(picobringup
  pico_stdlib
  hardware_gpio
  hardware_i2c
  pico_i2c_slave
)