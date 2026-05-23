# Bidin Firmware - Minimal ESP32-S3 Voice Assistant
cmake_minimum_required(VERSION 3.16)

# Set target chip (ESP32-S3)
set(IDF_TARGET "esp32s3")

# Project name
project(bidin_firmware VERSION 1.0.0 LANGUAGES C CXX ASM)

# Include ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Build the main application
idf_build_component(.)

# Create binary
idf_build_executable(bidin_firmware)
