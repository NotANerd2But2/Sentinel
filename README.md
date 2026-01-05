# Sentinel System Monitor

A high-assurance Windows process monitoring system built with modern C++20, leveraging native Windows APIs for deep system integration.

## Overview

Sentinel System Monitor embodies a philosophy of deep integration with the Windows operating system, leveraging native APIs and kernel-mode interfaces to achieve maximum visibility and control. This project monitors target processes from User-Mode (Ring 3) to detect instability, unauthorized handles, and memory corruption.

## Phase 1: Build System & Logger ✅

This initial phase establishes the project infrastructure with:
- CMake-based build system configured for Windows x64
- Thread-safe colored console logger
- Comprehensive Doxygen documentation

## Requirements

- **Platform:** Windows 10/11 (x64 only)
- **Compiler:** Microsoft Visual C++ (MSVC) with C++20 support
- **Build System:** CMake 3.20 or later
- **IDE:** Visual Studio 2019/2022 (recommended)

## Building the Project

### Using Visual Studio

1. Open the project folder in Visual Studio
2. Visual Studio will automatically detect the CMake configuration
3. Select the build configuration (Debug/Release)
4. Build the solution (Ctrl+Shift+B)

### Using Command Line

```cmd
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release
```

## Project Structure

```
Sentinel/
├── CMakeLists.txt              # Root build configuration
├── cmake/
│   └── CompilerWarnings.cmake  # MSVC warning flags
├── src/
│   ├── CMakeLists.txt          # Source build configuration
│   ├── Sentinel/
│   │   └── Utils/
│   │       ├── Logger.hpp      # Logger interface
│   │       └── Logger.cpp      # Logger implementation
│   └── main.cpp                # Test executable
├── tests/
│   └── CMakeLists.txt          # Test configuration
└── docs/
    └── ARCHITECTURE.md         # System architecture specification
```

## Logger Usage

The Logger module provides thread-safe, color-coded console output:

```cpp
#include "Sentinel/Utils/Logger.hpp"

using namespace Sentinel::Utils;

int main() {
    // Log informational message (green text)
    Logger::LogInfo("Sentinel monitor initialized successfully");
    
    // Log error message (red text)
    Logger::LogError("Failed to attach to target process");
    
    return 0;
}
```

### Features

- **Thread-safe:** Uses std::mutex for concurrent access protection
- **Color-coded:** Green for info, red for errors
- **Stream separation:** Info to stdout, errors to stderr
- **Graceful degradation:** Works without colors if console unavailable

## Build Configuration

### Compiler Flags

The project enforces strict MSVC compilation standards:
- `/W4` - Warning level 4 (highest standard level)
- `/WX` - Treat warnings as errors
- `/permissive-` - Enforce standards conformance
- `/Zc:__cplusplus` - Enable updated __cplusplus macro

### Platform Validation

The build system performs strict platform validation:
- Windows operating system required (FATAL_ERROR on Linux/macOS)
- x64 architecture required (FATAL_ERROR on x86)

## Development

### Code Style

- Modern C++20 features encouraged
- Comprehensive Doxygen documentation required
- Explain the *why*, not just the *what*
- Follow ARCHITECTURE.md guidelines

### Documentation

All header files must include full Doxygen comments explaining:
- **Rationale:** Why the class/function exists
- **Design decisions:** Why this approach was chosen
- **Usage patterns:** How to use the component correctly
- **Security implications:** Potential security considerations
- **Performance characteristics:** Time/space complexity notes

## Testing

Run the test executable to verify the logger:

```cmd
# From build directory
Release\SentinelTest.exe
```

This demonstrates:
- Single-threaded logging
- Multi-threaded concurrent access
- Color output functionality

## Next Steps

Upcoming phases will implement:
- **Module A:** Crash Interceptor (Vectored Exception Handling)
- **Module B:** Resource Auditor (Handle enumeration)
- **Module C:** Integrity Engine (Stack-based VM)
- **Module D:** Communication Pipe (Named pipes with SDDL)

## License

See repository for license information.

## References

- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - Detailed system architecture
- [Windows Console API](https://docs.microsoft.com/en-us/windows/console/console-functions)
