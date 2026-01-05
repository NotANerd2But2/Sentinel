/**
 * @file CrashInterceptor.cpp
 * @brief Implementation of Vectored Exception Handler for crash interception.
 */

#include "Sentinel/Bedrock/CrashInterceptor.hpp"
#include "Sentinel/Utils/Logger.hpp"
#include <stdio.h>

namespace Sentinel {
namespace Bedrock {

// Windows exception codes (defined explicitly to ensure availability across all build configurations)
#ifndef STATUS_GUARD_PAGE_VIOLATION
#define STATUS_GUARD_PAGE_VIOLATION 0x80000001L
#endif

#ifndef STATUS_ACCESS_VIOLATION
#define STATUS_ACCESS_VIOLATION 0xC0000005L
#endif

// Page size mask for address sanitization (4KB = 0x1000, mask = 0xFFF)
// Masking lower 12 bits aligns addresses to page boundaries to prevent ASLR bypass
static constexpr uintptr_t PAGE_OFFSET_MASK = 0xFFFULL;

bool CrashInterceptor::Initialize() {
    // Register the Vectored Exception Handler with priority 1
    // Priority 1 ensures we execute before most handlers but after critical system handlers
    PVOID handler = AddVectoredExceptionHandler(1, HandlerRoutine);
    
    // Check if registration was successful
    if (handler == nullptr) {
        Utils::Logger::LogError("Failed to register Vectored Exception Handler");
        return false;
    }
    
    Utils::Logger::LogInfo("Crash Interceptor initialized successfully");
    return true;
}

LONG WINAPI CrashInterceptor::HandlerRoutine(PEXCEPTION_POINTERS ExceptionInfo) {
    // Validate exception information pointer
    if (ExceptionInfo == nullptr || ExceptionInfo->ExceptionRecord == nullptr) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    
    // Extract exception code for analysis
    DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
    
    // Handle STATUS_GUARD_PAGE_VIOLATION (0x80000001)
    // This exception occurs when code accesses a guard page protected memory region
    // In the Sentinel architecture, this is used by the Integrity Engine for JIT decryption
    if (exceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        // Extract the faulting address from the exception record
        // For guard page violations, ExceptionInformation[1] contains the virtual address
        PVOID faultingAddress = nullptr;
        if (ExceptionInfo->ExceptionRecord->NumberParameters >= 2) {
            faultingAddress = reinterpret_cast<PVOID>(
                ExceptionInfo->ExceptionRecord->ExceptionInformation[1]
            );
        }
        
        // Sanitize address to prevent ASLR bypass: mask lower 12 bits (4KB page boundary)
        // This provides forensic information while protecting address space layout
        uintptr_t sanitizedAddress = reinterpret_cast<uintptr_t>(faultingAddress) & ~PAGE_OFFSET_MASK;
        
        // Format log message using stack-based buffer to avoid heap allocation in exception handler
        char logBuffer[256];
        int result = sprintf_s(logBuffer, sizeof(logBuffer),
                               "[CRITICAL] Guard Page Violation Detected at 0x%016llX (page-aligned)!",
                               static_cast<unsigned long long>(sanitizedAddress));
        
        // Only log if formatting succeeded
        if (result > 0) {
            Utils::Logger::LogError(logBuffer);
        } else {
            // Fallback message if formatting fails (should never happen with static format)
            Utils::Logger::LogError("[CRITICAL] Guard Page Violation Detected (formatting error)!");
        }
        
        // NOTE: This is where JIT decryption logic will be implemented in the future.
        // The JIT decryption process will:
        // 1. Validate that the faulting address is within the VM bytecode region
        // 2. Temporarily remove PAGE_GUARD protection from the faulting page
        // 3. Decrypt a single VM instruction into a secure execution buffer
        // 4. Allow execution to continue (return EXCEPTION_CONTINUE_EXECUTION)
        // 5. After instruction execution, restore PAGE_GUARD protection
        //
        // For now, we simply log the violation and continue the search chain.
        
        return EXCEPTION_CONTINUE_SEARCH;
    }
    
    // Handle STATUS_ACCESS_VIOLATION
    // This exception indicates illegal memory access (null pointer, buffer overflow, etc.)
    if (exceptionCode == STATUS_ACCESS_VIOLATION) {
        // Extract access violation details
        // ExceptionInformation[0]: 0 = read, 1 = write, 8 = DEP violation
        // ExceptionInformation[1]: Virtual address of inaccessible data
        PVOID faultingAddress = nullptr;
        ULONG_PTR accessType = 0;
        
        if (ExceptionInfo->ExceptionRecord->NumberParameters >= 2) {
            accessType = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
            faultingAddress = reinterpret_cast<PVOID>(
                ExceptionInfo->ExceptionRecord->ExceptionInformation[1]
            );
        }
        
        // Sanitize address to prevent ASLR bypass: mask lower 12 bits (4KB page boundary)
        // This provides forensic information while protecting address space layout
        uintptr_t sanitizedAddress = reinterpret_cast<uintptr_t>(faultingAddress) & ~PAGE_OFFSET_MASK;
        
        // Decode access type string
        const char* accessTypeStr = "Access to";
        switch (accessType) {
            case 0:
                accessTypeStr = "Read from";
                break;
            case 1:
                accessTypeStr = "Write to";
                break;
            case 8:
                accessTypeStr = "DEP violation at";
                break;
            default:
                accessTypeStr = "Access to";
                break;
        }
        
        // Format log message using stack-based buffer to avoid heap allocation in exception handler
        char logBuffer[256];
        int result = sprintf_s(logBuffer, sizeof(logBuffer),
                               "[CRITICAL] Access Violation! %s address 0x%016llX (page-aligned)",
                               accessTypeStr,
                               static_cast<unsigned long long>(sanitizedAddress));
        
        // Only log if formatting succeeded
        if (result > 0) {
            Utils::Logger::LogError(logBuffer);
        } else {
            // Fallback message if formatting fails (should never happen with static format)
            Utils::Logger::LogError("[CRITICAL] Access Violation (formatting error)!");
        }
        
        // Continue the exception search chain
        // This allows the application's normal exception handling to proceed
        return EXCEPTION_CONTINUE_SEARCH;
    }
    
    // For all other exception types, immediately continue the search
    // This ensures minimal overhead for expected exceptions
    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace Bedrock
} // namespace Sentinel
