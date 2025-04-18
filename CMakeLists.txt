cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME CmakeTest)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_SOURCE_DIR}/Core/Src/Log.c
    ${CMAKE_SOURCE_DIR}/Core/Src/Constrain.c
    ${CMAKE_SOURCE_DIR}/Modules/Tasks/RFTask/RF_Task.c
    ${CMAKE_SOURCE_DIR}/Modules/Tasks/MainTask/Main_task.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/Src/radio_user.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/lr_fhss_mac.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/ral_sx126x_bsp.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/ral_sx126x.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/ralf_sx126x.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/sx126x_hal.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/sx126x_lr_fhss.c
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Src/sx126x.c
    ${CMAKE_SOURCE_DIR}/Modules/ATInterface/SerialPort/portSTM32L071xx.c
    ${CMAKE_SOURCE_DIR}/Modules/ATInterface/AT_cmd.c
    ${CMAKE_SOURCE_DIR}/Modules/Tasks/MainTask/general_sys_cmd.c
    ${CMAKE_SOURCE_DIR}/Modules/Tasks/MainTask/auxPin_logic.c
    ${CMAKE_SOURCE_DIR}/Modules/NVMA/NVMA.c
    
    
    
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_SOURCE_DIR}/Inc
    ${CMAKE_SOURCE_DIR}/Modules/RF/Inc
    ${CMAKE_SOURCE_DIR}/Modules/RF/SX1262/Inc
    ${CMAKE_SOURCE_DIR}/Modules/ATInterface/SerialPort
    ${CMAKE_SOURCE_DIR}/Modules/ATInterface/
    ${CMAKE_SOURCE_DIR}/Modules/NVMA
    ${CMAKE_SOURCE_DIR}/Modules/Tasks/MainTask
    ${CMAKE_SOURCE_DIR}/Modules/Tasks/RFTask
    
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
