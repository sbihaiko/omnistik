cmake_minimum_required(VERSION 3.13)

# Spike de diagnóstico: HID mínimo, sem stdio e sem envio de relatório.

set(PICO_BOARD pico2_w CACHE STRING "Placa alvo")

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "defina PICO_SDK_PATH")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(hid_min C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(OMNISTIK_PADS 1 CACHE STRING "Número de interfaces HID de gamepad")

add_executable(hid_min
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(hid_min PRIVATE src)
target_compile_definitions(hid_min PRIVATE OMNISTIK_PAD_COUNT=${OMNISTIK_PADS})
target_link_libraries(hid_min
    pico_stdlib
    tinyusb_device
)

pico_enable_stdio_usb(hid_min 0)
pico_enable_stdio_uart(hid_min 0)

pico_add_extra_outputs(hid_min)
