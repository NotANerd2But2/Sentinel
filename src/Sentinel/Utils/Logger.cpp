/**
 * @file Logger.cpp
 * @brief Implementation of thread-safe console logger with colored output.
 */

#include "Sentinel/Utils/Logger.hpp"
#include <iostream>

namespace Sentinel {
namespace Utils {

// Static member initialization
std::mutex Logger::consoleMutex_;
HANDLE Logger::consoleHandle_ = nullptr;
HANDLE Logger::errorConsoleHandle_ = nullptr;
WORD Logger::defaultAttributes_ = 0;
WORD Logger::errorDefaultAttributes_ = 0;
bool Logger::initialized_ = false;

// Default console color attributes (white text on black background)
static constexpr WORD DEFAULT_CONSOLE_ATTRIBUTES = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

bool Logger::IsConsoleAvailable(HANDLE handle) {
    return (handle != INVALID_HANDLE_VALUE && handle != nullptr);
}

void Logger::Initialize() {
    if (!initialized_) {
        // Get handle to standard output
        consoleHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Get handle to standard error (for proper handling when streams are redirected separately)
        errorConsoleHandle_ = GetStdHandle(STD_ERROR_HANDLE);
        
        // Get default console attributes for stdout
        if (IsConsoleAvailable(consoleHandle_)) {
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            if (GetConsoleScreenBufferInfo(consoleHandle_, &consoleInfo)) {
                defaultAttributes_ = consoleInfo.wAttributes;
            } else {
                // Fallback to default console attributes
                defaultAttributes_ = DEFAULT_CONSOLE_ATTRIBUTES;
            }
        } else {
            // If stdout console is not available, set default attributes
            defaultAttributes_ = DEFAULT_CONSOLE_ATTRIBUTES;
        }
        
        // Get default console attributes for stderr
        if (IsConsoleAvailable(errorConsoleHandle_)) {
            CONSOLE_SCREEN_BUFFER_INFO errorConsoleInfo;
            if (GetConsoleScreenBufferInfo(errorConsoleHandle_, &errorConsoleInfo)) {
                errorDefaultAttributes_ = errorConsoleInfo.wAttributes;
            } else {
                // Fallback to default console attributes
                errorDefaultAttributes_ = DEFAULT_CONSOLE_ATTRIBUTES;
            }
        } else {
            // If stderr console is not available, set default attributes
            errorDefaultAttributes_ = DEFAULT_CONSOLE_ATTRIBUTES;
        }
        
        initialized_ = true;
    }
}

void Logger::LogInfo(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex_);
    
    // Initialize console on first use
    if (!initialized_) {
        Initialize();
    }
    
    // Set green color (bright green) if console is available
    if (IsConsoleAvailable(consoleHandle_)) {
        SetConsoleTextAttribute(consoleHandle_, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }
    
    // Output message to stdout
    std::cout << "[INFO] " << message << std::endl;
    
    // Restore default color if console is available
    if (IsConsoleAvailable(consoleHandle_)) {
        SetConsoleTextAttribute(consoleHandle_, defaultAttributes_);
    }
}

void Logger::LogError(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex_);
    
    // Initialize console on first use
    if (!initialized_) {
        Initialize();
    }
    
    // Set red color (bright red) if console is available
    // Use stderr handle for proper handling when streams are redirected separately
    if (IsConsoleAvailable(errorConsoleHandle_)) {
        SetConsoleTextAttribute(errorConsoleHandle_, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    
    // Output message to stderr
    std::cerr << "[ERROR] " << message << std::endl;
    
    // Restore default color if console is available
    if (IsConsoleAvailable(errorConsoleHandle_)) {
        SetConsoleTextAttribute(errorConsoleHandle_, errorDefaultAttributes_);
    }
}

} // namespace Utils
} // namespace Sentinel
