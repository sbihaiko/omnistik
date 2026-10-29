cmake_minimum_required(VERSION 3.13)

# Spike de diagnóstico: a placa enumera como USB serial (CDC) usando a stdio
# padrão do Pico SDK? Se sim, o USB da placa está bom e o problema está nos
# descritores de gamepad.

set(PICO_BOARD pico2_w CACHE STRING "Placa alvo")

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "defina PICO_SDK_PATH")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(usb_hello C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(usb_hello
    src/main.c
)

target_link_libraries(usb_hello pico_stdlib)

pico_enable_stdio_usb(usb_hello 1)
pico_enable_stdio_uart(usb_hello 0)

pico_add_extra_outputs(usb_hello)
