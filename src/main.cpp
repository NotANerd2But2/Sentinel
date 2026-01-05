/**
 * @file main.cpp
 * @brief Simple test application to demonstrate the Sentinel Logger functionality.
 */

#include "Sentinel/Utils/Logger.hpp"
#include <thread>
#include <chrono>

int main() {
    using namespace Sentinel::Utils;
    
    // Test basic logging
    Logger::LogInfo("Sentinel System Monitor - Build System Test");
    Logger::LogInfo("Testing thread-safe logger with colored output");
    
    // Test error logging
    Logger::LogError("This is a test error message");
    
    // Test multi-threaded logging
    Logger::LogInfo("Starting multi-threaded test...");
    
    auto threadFunc = [](int id) {
        for (int i = 0; i < 3; ++i) {
            Logger::LogInfo("Thread " + std::to_string(id) + " - Message " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };
    
    std::thread t1(threadFunc, 1);
    std::thread t2(threadFunc, 2);
    std::thread t3(threadFunc, 3);
    
    t1.join();
    t2.join();
    t3.join();
    
    Logger::LogInfo("Multi-threaded test completed successfully");
    Logger::LogInfo("Logger demonstration complete");
    
    return 0;
}
