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
WORD Logger::defaultAttributes_ = 0;
bool Logger::initialized_ = false;

void Logger::Initialize() {
    if (!initialized_) {
        // Get handle to standard output
        consoleHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
        
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
    
    // Set green color (bright green)
    SetConsoleTextAttribute(consoleHandle_, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    
    // Output message
    std::cout << "[INFO] " << message << std::endl;
    
    // Restore default color
    SetConsoleTextAttribute(consoleHandle_, defaultAttributes_);
}

void Logger::LogError(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex_);
    
    // Initialize console on first use
    if (!initialized_) {
        Initialize();
    }
    
    // Set red color (bright red)
    SetConsoleTextAttribute(consoleHandle_, FOREGROUND_RED | FOREGROUND_INTENSITY);
    
    // Output message
    std::cout << "[ERROR] " << message << std::endl;
    
    // Restore default color
    SetConsoleTextAttribute(consoleHandle_, defaultAttributes_);
}

} // namespace Utils
} // namespace Sentinel
