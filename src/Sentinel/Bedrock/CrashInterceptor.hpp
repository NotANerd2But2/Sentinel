/**
 * @file CrashInterceptor.hpp
 * @brief Vectored Exception Handler for crash interception and system stability monitoring.
 * 
 * @details This module exists to provide early detection of process instability through
 * Windows Vectored Exception Handling (VEH). Unlike traditional Structured Exception
 * Handling (SEH), VEH allows Sentinel to intercept exceptions before the standard handler
 * chain is invoked, enabling custom recovery logic, forensic analysis, and Just-In-Time
 * (JIT) decryption for the virtualized integrity engine.
 * 
 * The CrashInterceptor serves as the foundational layer of the Sentinel System Monitor,
 * implementing "Module A: The Crash Interceptor (Bedrock)" as described in the system
 * architecture. Its primary purpose is System Stability Monitoring - detecting and
 * responding to critical exceptions that indicate process instability or security threats.
 * 
 * Key responsibilities:
 * - Intercept STATUS_ACCESS_VIOLATION exceptions indicating memory corruption or null dereferences
 * - Intercept STATUS_GUARD_PAGE_VIOLATION exceptions from guard page protection mechanisms
 * - Prepare for future JIT decryption logic for the virtualized integrity engine
 * - Preserve exception context for forensic analysis
 * - Enable custom recovery logic while maintaining process stability
 * 
 * The design prioritizes:
 * - Early detection: VEH registration provides first-responder status for all exceptions
 * - Low overhead: One-time registration cost with minimal per-exception overhead
 * - Context preservation: Full CPU state (CONTEXT) and exception details (EXCEPTION_RECORD) capture
 * - Security integration: Guard page violations are monitored for the integrity engine's JIT decryption
 * - Forensic capability: Detailed logging of exception addresses and types for incident response
 * 
 * @security This handler executes in the context of exception handling and has access to
 * the complete process state including CPU registers, stack contents, and exception details.
 * Care must be taken to:
 * - Prevent information disclosure through logging (sanitize addresses in production)
 * - Avoid recursive exceptions within the handler itself
 * - Maintain handler stability to prevent cascading failures
 * - Protect handler logic from tampering or bypass attempts
 * 
 * @performance The exception handler is invoked on every exception in the process, including
 * expected exceptions from the CLR, system libraries, and application code. The handler must
 * execute with minimal latency to avoid impacting process performance. Current implementation:
 * - O(1) exception code comparison
 * - Direct API calls with no dynamic allocation
 * - Fast path for non-critical exceptions (EXCEPTION_CONTINUE_SEARCH)
 * 
 * @see https://docs.microsoft.com/en-us/windows/win32/debug/vectored-exception-handling
 * @see ARCHITECTURE.md Section 2: Module A - The Crash Interceptor
 */

#pragma once

#include <Windows.h>

namespace Sentinel {
namespace Bedrock {

/**
 * @class CrashInterceptor
 * @brief Manages Vectored Exception Handling for system stability monitoring.
 * 
 * @details The CrashInterceptor class provides a high-level interface for registering
 * and managing a Vectored Exception Handler that monitors critical exceptions indicating
 * process instability. This is the foundational component (Bedrock) of the Sentinel System.
 * 
 * The class handles:
 * - VEH registration through the Windows API
 * - Exception filtering based on exception codes
 * - Logging of critical exceptions for forensic analysis
 * - Preparation for future JIT decryption logic (guard page handling)
 * 
 * Usage example:
 * @code
 * Sentinel::Bedrock::CrashInterceptor interceptor;
 * if (interceptor.Initialize()) {
 *     // VEH is now active and monitoring exceptions
 *     // The handler will intercept critical exceptions throughout process lifetime
 * }
 * @endcode
 * 
 * @note This class is designed for single initialization. Multiple instances may
 * register multiple handlers, but typically only one instance is needed per process.
 * 
 * @note The handler remains active for the lifetime of the process unless explicitly
 * removed. Current implementation does not provide unregistration to ensure continuous
 * monitoring.
 */
class CrashInterceptor {
public:
    /**
     * @brief Initializes the Crash Interceptor by registering a Vectored Exception Handler.
     * 
     * @details Registers a Vectored Exception Handler with Windows using the
     * AddVectoredExceptionHandler API. The handler is registered with priority 1,
     * ensuring it executes before most other exception handlers but after critical
     * system handlers (priority 0).
     * 
     * The handler will intercept all exceptions in the process and filter for:
     * - STATUS_GUARD_PAGE_VIOLATION (0x80000001): Guard page protection violations
     * - STATUS_ACCESS_VIOLATION: Memory access violations
     * 
     * @return true if the VEH was successfully registered, false otherwise.
     * 
     * @note This method should be called once during application initialization.
     * Calling it multiple times will register multiple handlers with the same routine.
     * 
     * @note The handler remains registered for the lifetime of the process. There is
     * no corresponding Shutdown() method as continuous monitoring is required.
     * 
     * @performance Registration is a one-time operation with minimal overhead.
     * Typical execution time is < 1ms.
     * 
     * @threadsafe This method is not thread-safe. It should be called from a single
     * thread during initialization before multi-threaded execution begins.
     */
    bool Initialize();

private:
    /**
     * @brief Vectored Exception Handler routine for crash interception.
     * 
     * @details This static callback function is invoked by Windows whenever an exception
     * occurs in the process. It serves as the entry point for exception analysis and
     * handles critical exceptions that indicate process instability or security events.
     * 
     * Exception handling logic:
     * 
     * 1. STATUS_GUARD_PAGE_VIOLATION (0x80000001):
     *    - Occurs when code attempts to access a memory page protected with PAGE_GUARD
     *    - In Sentinel, this is used by the Integrity Engine for JIT decryption
     *    - Currently logs the violation and returns EXCEPTION_CONTINUE_SEARCH
     *    - Future: Will implement JIT decryption logic to temporarily remove guard protection,
     *      decrypt a single VM instruction, execute it, and restore protection
     * 
     * 2. STATUS_ACCESS_VIOLATION:
     *    - Indicates illegal memory access (null pointer, buffer overflow, etc.)
     *    - Logs the violation with sanitized address for forensic analysis
     *    - Returns EXCEPTION_CONTINUE_SEARCH to allow normal exception handling
     * 
     * 3. All other exceptions:
     *    - Returns EXCEPTION_CONTINUE_SEARCH immediately to minimize overhead
     * 
     * Address Sanitization:
     * - All logged addresses are masked to 4KB page boundaries (lower 12 bits cleared)
     * - Prevents ASLR bypass while maintaining forensic value
     * - Addresses are logged as "page-aligned" to indicate sanitization
     * 
     * @param ExceptionInfo Pointer to EXCEPTION_POINTERS structure containing:
     *        - ExceptionRecord: Details about the exception (code, address, parameters)
     *        - ContextRecord: Complete CPU register state at time of exception
     * 
     * @return LONG value indicating how Windows should proceed:
     *         - EXCEPTION_CONTINUE_SEARCH: Continue to next exception handler in chain
     *         - EXCEPTION_CONTINUE_EXECUTION: Resume execution (not currently used)
     * 
     * @note This function executes in the context of exception handling. It must:
     *       - Execute quickly to avoid performance degradation
     *       - Avoid operations that could themselves trigger exceptions
     *       - Not perform dynamic memory allocation (heap may be corrupted)
     *       - Use thread-safe logging mechanisms
     * 
     * @implementation Uses sprintf_s with stack-based buffers to avoid heap allocation.
     *                 All string formatting is done on the stack for maximum performance
     *                 and safety in the exception handling context.
     * 
     * @security This handler has access to the complete process state. All addresses
     *           are sanitized to page boundaries before logging to prevent ASLR bypass.
     *           Uses sprintf_s to prevent buffer overflows.
     * 
     * @performance This function is called on every exception. Current implementation
     *              uses fast-path logic with O(1) comparisons, stack-based formatting,
     *              and immediate return for non-critical exceptions.
     * 
     * @threadsafe This function may be called concurrently from multiple threads if
     *             exceptions occur simultaneously. All operations must be thread-safe.
     * 
     * @see https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-addvectoredexceptionhandler
     */
    static LONG WINAPI HandlerRoutine(PEXCEPTION_POINTERS ExceptionInfo);
};

} // namespace Bedrock
} // namespace Sentinel
