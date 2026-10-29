cmake_minimum_required(VERSION 3.13)

# Sonda de saída (spec #1): quantos jogadores um único Pico 2 W entrega ao
# destino, enumerando N interfaces HID de gamepad num só dispositivo USB.
#
# Build:
#   cmake -B build -DOMNISTIK_PADS=1        # onda 1
#   cmake -B build -DOMNISTIK_PADS=2        # onda 3
#   cmake -B build -DOMNISTIK_PADS=4        # onda 4

set(PICO_BOARD pico2_w CACHE STRING "Placa alvo")

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "defina PICO_SDK_PATH")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(hid_probe C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(OMNISTIK_PADS 1 CACHE STRING "Número de interfaces HID de gamepad (1..4)")
if(OMNISTIK_PADS LESS 1 OR OMNISTIK_PADS GREATER 4)
    message(FATAL_ERROR "OMNISTIK_PADS deve ficar entre 1 e 4")
endif()

add_executable(hid_probe
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(hid_probe PRIVATE src)
target_compile_definitions(hid_probe PRIVATE
    OMNISTIK_PAD_COUNT=${OMNISTIK_PADS}
)
target_link_libraries(hid_probe
    pico_stdlib
    tinyusb_device
)

# Sem CDC e sem UART: o dispositivo deve aparecer só como gamepads.
pico_enable_stdio_usb(hid_probe 0)
pico_enable_stdio_uart(hid_probe 0)

pico_add_extra_outputs(hid_probe)
