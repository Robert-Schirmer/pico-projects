# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(plantingtosh C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)
set(WIFI_SSID $ENV{WIFI_SSID})
set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})

if (NOT WIFI_SSID)
    message(FATAL_ERROR "Please set WIFI_SSID environment variable: export WIFI_SSID=your_ssid")
elseif(NOT WIFI_PASSWORD)
    message(FATAL_ERROR "Please set WIFI_PASSWORD environment variable: export WIFI_PASSWORD=your_password")
endif()


# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    main.c
    ssd1306.c
    tcp_client.c
    bootselbtn.c
    plant_stats.c
    mem_helpers.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE WIFI_SSID=${WIFI_SSID})
target_compile_definitions(${PROJECT_NAME} PRIVATE WIFI_PASSWORD=${WIFI_PASSWORD})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_cyw43_arch_lwip_poll
    pico_stdlib
    hardware_i2c
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
