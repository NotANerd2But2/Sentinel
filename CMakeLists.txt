cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Enforce Windows x64 platform (before project() to avoid toolchain detection issues)
if(NOT WIN32)
    message(FATAL_ERROR "This project is designed for Windows x64 only.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires x64 (64-bit) architecture.")
endif()

project(SentinelMonitor VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable folder organization in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include compiler warnings
include(cmake/CompilerWarnings.cmake)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)
