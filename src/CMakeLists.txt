# SentinelCore static library

# Collect source files
set(SENTINEL_SOURCES
    Sentinel/Utils/Logger.cpp
)

set(SENTINEL_HEADERS
    Sentinel/Utils/Logger.hpp
)

# Create static library
add_library(SentinelCore STATIC
    ${SENTINEL_SOURCES}
    ${SENTINEL_HEADERS}
)

# Set include directories
target_include_directories(SentinelCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link Windows libraries
target_link_libraries(SentinelCore
    PUBLIC
        Kernel32
        User32
)

# Organize files in IDE
source_group("Source Files\\Utils" FILES Sentinel/Utils/Logger.cpp)
source_group("Header Files\\Utils" FILES Sentinel/Utils/Logger.hpp)

# Create test executable (for demonstration purposes)
add_executable(SentinelTest main.cpp)
target_link_libraries(SentinelTest PRIVATE SentinelCore)
