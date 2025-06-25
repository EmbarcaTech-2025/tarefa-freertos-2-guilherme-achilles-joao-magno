# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")
SET(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS)

message("FreeRTOS Kernel located in ${FREERTOS_PATH}")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)


project(projeto_grupo_x C CXX ASM)


# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(projeto_grupo_x src/main.c)

add_library(connections STATIC 
    connections/wifi_control.c 
    connections/mqtt_comm.c 
    connections/criptografias.c
)

add_library(driver STATIC 
    driver/ssd1306.c
    driver/temperature_sensor.c
    driver/button.c
)
add_library(hal STATIC
    hal/write_display.c
    hal/temp_transform.c
    hal/neopixel_pio.c
)
add_library(termometro_dinamica STATIC
    termometro_dinamica/visor_dinamica.c
)


pico_set_program_name(projeto_grupo_x "projeto_grupo_x")
pico_set_program_version(projeto_grupo_x "0.1")
pico_generate_pio_header(hal  ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(projeto_grupo_x 0)
pico_enable_stdio_usb(projeto_grupo_x 1)

# Add the standard library to the build
target_link_libraries(projeto_grupo_x
        pico_stdlib
        FreeRTOS-Kernel-Heap4
        hardware_i2c
        driver         
        hal
        pico_stdlib
        termometro_dinamica
        connections
)

target_link_libraries(driver 
        pico_stdlib
        hardware_i2c
        hardware_pwm
        hardware_adc
        hardware_irq
        hardware_dma
        hal
    )

target_link_libraries(hal
        pico_stdlib
        hardware_i2c
        hardware_pio
        hardware_clocks
        driver
    )

target_link_libraries(termometro_dinamica
        pico_stdlib
        hal
    )

target_link_libraries(connections
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        pico_cyw43_driver
        pico_mbedtls
        hal
        driver
)


target_include_directories(projeto_grupo_x PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_include_directories(hal PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_include_directories(driver PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_include_directories(termometro_dinamica PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_include_directories(connections PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_add_extra_outputs(projeto_grupo_x)

