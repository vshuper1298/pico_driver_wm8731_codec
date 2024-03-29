cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project) and extras
include(${CMAKE_SOURCE_DIR}/pico-sdk/external/pico_sdk_import.cmake)
include(${CMAKE_SOURCE_DIR}/pico-extras/external/pico_extras_import.cmake)

project(adc_driver C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
# set(CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++)

set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel")

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

# Add FreeRTOS as a library
add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0/port.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
    ${EXAMPLES}/
    ${EXAMPLES}/pio/
)

set(EXAMPLES
    examples/pio/pio_example.cpp
)

set(SOURCES 
    hardware.c
    hardware.h
    dmac.c
    dmac.h
    wm8731.h
    wm8731.c
    # wm8731_unit_test.c
)

add_executable(
    adc_driver
    main.cpp
    ${SOURCES}
    ${EXAMPLES}
)

pico_generate_pio_header(adc_driver ${CMAKE_CURRENT_LIST_DIR}/pio_serialiser.pio)
pico_generate_pio_header(adc_driver ${CMAKE_CURRENT_LIST_DIR}/examples/pio/example.pio)

# pull in common dependencies
target_link_libraries(
    adc_driver
    pico_stdlib
    hardware_dma
    hardware_adc
    hardware_irq
    hardware_pio
    pico_audio_i2s
    pico_multicore
    pico_sync
)

target_compile_definitions(adc_driver PRIVATE
        # compile time configuration of I2S
        PICO_AUDIO_I2S_MONO_INPUT=1
        #define for our example code
        USE_AUDIO_I2S=1
        PICO_AUDIO_I2S_DATA_PIN=22
        PICO_AUDIO_I2S_CLOCK_PIN_BASE=24
        # PICO_DEFAULT_UART=0
        # PICO_DEFAULT_UART_TX_PIN=28
        # PICO_DEFAULT_UART_RX_PIN=29
)

# create map/bin/hex file etc.
pico_add_extra_outputs(adc_driver)

# # add url via pico_set_program_url
# example_auto_set_url(adc_driver)

pico_enable_stdio_usb(adc_driver 1)
pico_enable_stdio_uart(adc_driver 1)