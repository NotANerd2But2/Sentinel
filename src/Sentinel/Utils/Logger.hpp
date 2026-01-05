/**
 * @file Logger.hpp
 * @brief Thread-safe console logger with colored output for the Sentinel System.
 * 
 * @details This module exists to provide thread-safe, color-coded console logging
 * for the Sentinel System Monitor. In a multi-threaded monitoring environment,
 * concurrent log writes can produce garbled output. This logger uses mutex
 * synchronization to ensure atomic log operations while providing visual
 * distinction between information and error messages through Windows console
 * color attributes.
 * 
 * The design prioritizes:
 * - Thread safety: Mutex-protected console writes prevent race conditions
 * - Visual clarity: Color coding enables rapid identification of log severity
 * - Performance: Minimal overhead with single mutex and direct console API usage
 * - Windows integration: Native use of Windows Console API for color support
 * 
 * @security This logger writes to stdout/stderr and may expose sensitive
 * information. Care must be taken to sanitize log messages in production builds.
 * 
 * @performance The mutex synchronization introduces minimal overhead. However,
 * excessive logging in performance-critical paths should be avoided. Consider
 * conditional compilation or log level filtering for production builds.
 * 
 * @see https://docs.microsoft.com/en-us/windows/console/console-functions
 */

#pragma once

#include <string>
#include <mutex>
#include <Windows.h>

namespace Sentinel {
namespace Utils {

/**
 * @class Logger
 * @brief Thread-safe console logger with Windows console color support.
 * 
 * @details Provides static methods for logging information and error messages
 * to the Windows console with color-coded output. All methods are thread-safe
 * through internal mutex synchronization.
 * 
 * Usage example:
 * @code
 * Logger::LogInfo("Sentinel monitor initialized successfully");
 * Logger::LogError("Failed to attach to target process");
 * @endcode
 */
class Logger {
public:
    /**
     * @brief Logs an informational message to the console in green text.
     * 
     * @details Outputs the provided message to stdout with green color attribute
     * (FOREGROUND_GREEN | FOREGROUND_INTENSITY). This method is thread-safe and
     * can be called concurrently from multiple threads without risking garbled
     * output.
     * 
     * @param message The informational message to log.
     * 
     * @note The console color is automatically restored to the default after
     * the message is written to prevent color bleeding into subsequent output.
     * 
     * @performance Acquires a mutex lock, writes to console, and releases the lock.
     * Typical execution time is < 1ms unless console I/O is blocked.
     * 
     * @threadsafe This method is thread-safe.
     */
    static void LogInfo(const std::string& message);

    /**
     * @brief Logs an error message to the console in red text.
     * 
     * @details Outputs the provided message to stderr with red color attribute
     * (FOREGROUND_RED | FOREGROUND_INTENSITY). This method is thread-safe and
     * can be called concurrently from multiple threads without risking garbled
     * output. Error messages are sent to stderr to enable proper separation
     * from standard output and to support shell redirection.
     * 
     * @param message The error message to log.
     * 
     * @note The console color is automatically restored to the default after
     * the message is written to prevent color bleeding into subsequent output.
     * 
     * @performance Acquires a mutex lock, writes to console, and releases the lock.
     * Typical execution time is < 1ms unless console I/O is blocked.
     * 
     * @threadsafe This method is thread-safe.
     */
    static void LogError(const std::string& message);

private:
    /**
     * @brief Mutex for synchronizing console access across threads.
     * 
     * @details Protects console operations from concurrent access. This ensures
     * that log messages are written atomically and prevents interleaved output
     * from multiple threads.
     */
    static std::mutex consoleMutex_;

    /**
     * @brief Handle to the standard output console.
     * 
     * @details Retrieved via GetStdHandle(STD_OUTPUT_HANDLE) and used for
     * setting console text attributes. Cached as a static member to avoid
     * repeated API calls.
     */
    static HANDLE consoleHandle_;

    /**
     * @brief Handle to the standard error console.
     * 
     * @details Retrieved via GetStdHandle(STD_ERROR_HANDLE) and used for
     * setting console text attributes for error messages. Cached as a static
     * member to avoid repeated API calls.
     */
    static HANDLE errorConsoleHandle_;

    /**
     * @brief Default console text attributes.
     * 
     * @details Stores the original console color attributes to restore after
     * colored logging. Retrieved during static initialization via
     * GetConsoleScreenBufferInfo.
     */
    static WORD defaultAttributes_;

    /**
     * @brief Static initializer flag.
     * 
     * @details Ensures console handle and default attributes are initialized
     * only once. Set to true after first initialization.
     */
    static bool initialized_;

    /**
     * @brief Initializes console handle and default attributes.
     * 
     * @details Called automatically before first use of LogInfo or LogError.
     * Retrieves the console handle and saves the default text attributes for
     * later restoration.
     * 
     * @note This method is not thread-safe and relies on being called under
     * the consoleMutex_ lock.
     */
    static void Initialize();
};

} // namespace Utils
} // namespace Sentinel
