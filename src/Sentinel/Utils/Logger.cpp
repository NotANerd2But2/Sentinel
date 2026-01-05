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
bool Logger::initialized_ = false;

void Logger::Initialize() {
    if (!initialized_) {
        // Get handle to standard output
        consoleHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Get handle to standard error
        errorConsoleHandle_ = GetStdHandle(STD_ERROR_HANDLE);
        
        // Validate console handle
        if (consoleHandle_ == INVALID_HANDLE_VALUE || consoleHandle_ == nullptr) {
            // If console is not available, set default attributes and mark as initialized
            // Logging will continue without colors
            defaultAttributes_ = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            initialized_ = true;
            return;
        }
        
        // Get default console attributes
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        if (GetConsoleScreenBufferInfo(consoleHandle_, &consoleInfo)) {
            defaultAttributes_ = consoleInfo.wAttributes;
        } else {
            // Fallback to white text on black background if unable to retrieve
            defaultAttributes_ = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
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
    if (consoleHandle_ != INVALID_HANDLE_VALUE && consoleHandle_ != nullptr) {
        SetConsoleTextAttribute(consoleHandle_, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }
    
    // Output message
    std::cout << "[INFO] " << message << std::endl;
    
    // Restore default color if console is available
    if (consoleHandle_ != INVALID_HANDLE_VALUE && consoleHandle_ != nullptr) {
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
    if (errorConsoleHandle_ != INVALID_HANDLE_VALUE && errorConsoleHandle_ != nullptr) {
        SetConsoleTextAttribute(errorConsoleHandle_, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    
    // Output message to stderr
    std::cerr << "[ERROR] " << message << std::endl;
    
    // Restore default color if console is available
    if (errorConsoleHandle_ != INVALID_HANDLE_VALUE && errorConsoleHandle_ != nullptr) {
        SetConsoleTextAttribute(errorConsoleHandle_, defaultAttributes_);
    }
}

} // namespace Utils
} // namespace Sentinel
