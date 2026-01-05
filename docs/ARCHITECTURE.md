# Sentinel System Monitor - Architecture Specification

## 1. System Overview

### Philosophy
**"Native Windows Reliability & Introspection."**

The Sentinel System Monitor embodies a philosophy of deep integration with the Windows operating system, leveraging native APIs and kernel-mode interfaces to achieve maximum visibility and control. Rather than relying on third-party libraries or cross-platform abstractions, Sentinel embraces Windows-specific technologies to provide unparalleled insight into process health and system integrity.

### Core Goal
Monitor a target process from **User-Mode (Ring 3)** to detect instability, unauthorized handles, and memory corruption.

The Sentinel System operates entirely within user-mode while maintaining comprehensive oversight of target processes. By utilizing advanced Windows APIs and undocumented system calls, Sentinel can:

* **Detect instability** through exception monitoring and crash analysis
* **Identify unauthorized handles** from external processes attempting to manipulate the target
* **Discover memory corruption** via integrity checks and guard page violations
* **Provide actionable intelligence** for forensic analysis and incident response

This approach ensures high-assurance monitoring without requiring kernel-mode drivers, reducing complexity and maintaining compatibility across Windows 10 and Windows 11 systems.

---

## 2. Core Modules

### Module A: The Crash Interceptor (Bedrock)

#### Technology
**Vectored Exception Handling (VEH)**

#### Function
Intercept critical exceptions such as `STATUS_ACCESS_VIOLATION` and `STATUS_GUARD_PAGE_VIOLATION` to perform custom recovery through Just-In-Time (JIT) Analysis.

The Crash Interceptor serves as the foundational layer of the Sentinel System, providing the first line of defense against process instability. By registering a Vectored Exception Handler, Sentinel can intercept exceptions before they reach the standard Structured Exception Handling (SEH) chain, enabling:

* **Early detection** of access violations and memory protection faults
* **Context preservation** for forensic analysis
* **Custom recovery logic** to maintain process stability
* **JIT analysis** of the crash site for immediate threat assessment

#### Technical Reference
The module wraps the Windows API `AddVectoredExceptionHandler` to register a high-priority exception handler. When an exception occurs, the handler receives a pointer to an `EXCEPTION_POINTERS` structure, which contains:

* `EXCEPTION_RECORD`: Details about the exception type, address, and parameters
* `CONTEXT`: Complete CPU register state at the time of the exception

The handler parses these structures to extract critical information:

```
CONTEXT Structure Analysis:
- Instruction Pointer (RIP/EIP): Identifies the faulting code location
- Stack Pointer (RSP/ESP): Enables stack trace generation
- General Purpose Registers: Provides runtime state for analysis
- Flags Register: Indicates CPU state and execution mode
```

By analyzing the `CONTEXT` structures, Sentinel can determine:
* Whether the exception is recoverable
* The likely cause of the fault (null pointer, buffer overflow, heap corruption)
* Whether the exception represents a security threat or natural program flow

---

### Module B: The Resource Auditor (Internals)

#### Technology
**`NtQuerySystemInformation`** (Information Classes `0x10` and `0x40`)

#### Function
Enumerate open handles to the current process. Identify "Stale" or "Unauthorized" handles from external processes and sanitize them through handle closure or duplication.

The Resource Auditor provides deep visibility into the handle table, exposing which processes have opened handles to the monitored target. This capability is critical for detecting:

* **Process injection attempts** via remote thread creation
* **Memory manipulation** through debug privileges
* **Code injection** via write process memory handles
* **Unauthorized monitoring** by malicious processes

#### Technical Reference

The module utilizes undocumented Native API functions from `ntdll.dll`:

**Information Class `0x10` (SystemHandleInformation)**:
* Enumerates all handles in the system
* Returns handle type, process ID, and object address
* Enables correlation of handles to specific processes

**Information Class `0x40` (SystemExtendedHandleInformation)**:
* Provides extended handle information on 64-bit systems
* Includes handle access rights and object type index
* Supports large handle counts (>65535)

The sanitization process involves:

1. **Enumeration**: Query all system handles and filter for handles to the target process
2. **Classification**: Determine if each handle is authorized (expected) or unauthorized (suspicious)
3. **Analysis**: Inspect handle access rights to determine threat level
   * `PROCESS_VM_WRITE`: Indicates potential code injection capability
   * `PROCESS_CREATE_THREAD`: Suggests thread injection risk
   * `PROCESS_DUP_HANDLE`: Enables handle stealing attacks
4. **Sanitization**:
   * **Close**: Terminate unauthorized handles from malicious processes
   * **Duplicate**: Create a restricted duplicate with reduced permissions for auditing

This approach provides comprehensive handle hygiene, reducing the attack surface of the monitored process.

---

### Module C: The Integrity Engine (Virtualization)

#### Technology
**Stack-Based Virtual Machine (VM)**

#### Function
Execute proprietary integrity checks in a sandboxed environment, isolated from the main process execution context.

The Integrity Engine represents the most sophisticated component of the Sentinel System. By implementing a custom virtual machine, Sentinel can execute complex integrity verification logic while maintaining:

* **Code confidentiality**: Integrity check algorithms remain opaque to reverse engineers
* **Tamper resistance**: VM bytecode can be obfuscated and encrypted
* **Flexibility**: New integrity checks can be deployed without recompilation
* **Performance**: Optimized bytecode interpreter with JIT compilation support

#### Security Architecture

The VM employs multiple layers of security:

**Memory Encryption**:
* Bytecode is stored encrypted in memory using AES-256
* Decryption occurs only during execution within the VM context
* Encryption keys are derived from runtime entropy and process state

**Guard Page Protection**:
* VM bytecode regions are protected with `PAGE_GUARD` flags
* Any unauthorized access triggers a `STATUS_GUARD_PAGE_VIOLATION`
* The Crash Interceptor (Module A) handles guard page violations
* Legitimate VM execution temporarily removes guard protection during JIT decryption
* Guard pages are restored after instruction execution completes

**JIT Decryption Process**:
1. VM execution begins with encrypted bytecode buffer
2. Guard page exception handler validates the caller
3. Single instruction is decrypted into a secure execution buffer
4. Instruction executes in VM interpreter context
5. Execution buffer is wiped (cleared with secure memset)
6. Guard protection is restored on the encrypted buffer
7. Process repeats for next instruction

This architecture ensures that:
* Encrypted bytecode is never fully decrypted in memory
* Memory dumps cannot reveal integrity check algorithms
* Debuggers cannot easily trace VM execution flow
* Tampered VMs fail catastrophically, alerting operators

**VM Instruction Set**:
The stack-based VM supports instructions for:
* Memory reads and writes (with bounds checking)
* Process enumeration and validation
* Cryptographic operations (hashing, signature verification)
* Control flow (jumps, calls, returns)
* System information queries

---

### Module D: The Communication Pipe

#### Technology
**Named Pipes with Security Descriptor Definition Language (SDDL)**

#### Function
Provide secure, authenticated communication between the Monitor (client) and the Service (server).

The Communication Pipe establishes a trusted channel for:
* **Command and control**: Service sends monitoring directives to the Monitor
* **Telemetry reporting**: Monitor sends health data, alerts, and crash dumps to the Service
* **Configuration updates**: Dynamic reconfiguration without process restart
* **Authentication**: Mutual authentication using Windows security tokens

#### Security Design

**Named Pipe Configuration**:
* **Pipe Name**: `\\.\pipe\SentinelMonitor-{GUID}` (randomized GUID per installation)
* **Pipe Mode**: `PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT`
* **Max Instances**: Limited to prevent resource exhaustion
* **Buffer Size**: Optimized for low-latency message exchange

**SDDL Security Descriptor**:
The pipe employs a restrictive SDDL to enforce access control:

```
"D:(A;;GA;;;SY)(A;;GA;;;BA)(A;;GRGW;;;S-1-5-32-544)"
```

Breaking down the SDDL:
* `D:` - Discretionary ACL
* `(A;;GA;;;SY)` - Allow Generic All to Local System (SID: S-1-5-18)
* `(A;;GA;;;BA)` - Allow Generic All to Built-in Administrators (SID: S-1-5-32-544)
* `(A;;GRGW;;;S-1-5-32-544)` - Allow Generic Read/Write to specific authorized process

This configuration ensures:
* Only the Service (running as Local System) can create the pipe server
* Only authorized clients can connect to the pipe
* Unprivileged users and processes are denied access
* Impersonation is prevented through token validation

**Message Protocol**:
Communication follows a structured protocol:

1. **Handshake**: Client connects and sends authentication token
2. **Validation**: Server validates client identity and permissions
3. **Session Establishment**: Encrypted session key is negotiated
4. **Message Exchange**: Commands and responses are encrypted and authenticated
5. **Termination**: Clean disconnect with session teardown

**Thread Safety**:
The pipe implementation includes:
* Synchronization primitives (mutexes, events) for concurrent access
* Overlapped I/O for non-blocking communication
* Timeout handling to prevent deadlocks
* Error recovery for broken connections

---

## 3. Engineering Standards

### Language
**C++20 (Microsoft Visual C++ Compiler - MSVC)**

The Sentinel System is developed exclusively in modern C++20, leveraging the latest language features for:

* **Type safety**: Strong typing with concepts and constraints
* **Performance**: Zero-cost abstractions and compile-time optimization
* **Maintainability**: Enhanced readability with structured bindings, ranges, and coroutines
* **Windows Integration**: Native support for Windows APIs and COM interfaces

**Key C++20 Features in Use**:
* **Concepts**: Template constraints for API contracts
* **Ranges**: Simplified iteration and algorithm composition
* **Coroutines**: Asynchronous operations for pipe communication
* **Modules**: Improved compilation times and encapsulation (where supported)
* **Three-way comparison**: Simplified comparison operators

**MSVC-Specific Features**:
* **Structured Exception Handling**: Integration with Windows SEH
* **__declspec extensions**: DLL export/import, alignment, and thread-local storage
* **Intrinsics**: CPU-specific optimizations for cryptographic operations
* **SAL annotations**: Static analysis for code correctness

### Platform
**Windows x64 Only**

Sentinel is architected exclusively for 64-bit Windows platforms (Windows 10 and Windows 11), enabling:

* **Large address space**: Full 64-bit virtual memory addressing
* **Performance**: Native 64-bit register operations
* **Security**: Enhanced exploit mitigations (CFG, CET, ASLR)
* **API availability**: Access to modern Windows APIs

**Supported Configurations**:
* Windows 10 (Version 1809 and later)
* Windows 11 (All versions)
* x64 processor architecture (Intel/AMD)

**Unsupported Configurations**:
* Windows 7/8/8.1 (EOL operating systems)
* x86 (32-bit) architecture
* ARM64 (future consideration)
* Non-Windows platforms (Linux, macOS)

**Build Configuration**:
* **Optimization**: `/O2` (Maximum optimization)
* **Runtime Library**: `/MT` (Static CRT for standalone deployment)
* **Debugging**: `/Zi` (Full debug information)
* **Security**: `/guard:cf` (Control Flow Guard), `/CETCOMPAT` (CET compatibility)
* **Warnings**: `/W4` (High warning level), `/WX` (Warnings as errors)

### Documentation
**All headers (`.hpp`) must include full Doxygen comments explaining *why* a class exists.**

Documentation is a first-class concern in the Sentinel codebase. Every header file must contain comprehensive Doxygen comments that explain:

* **Rationale**: Why the class/function exists and what problem it solves
* **Design decisions**: Why this approach was chosen over alternatives
* **Usage patterns**: How the component should be used correctly
* **Security implications**: Potential security considerations
* **Performance characteristics**: Time/space complexity and optimization notes

**Doxygen Comment Structure**:

```cpp
/**
 * @file crash_interceptor.hpp
 * @brief Vectored Exception Handler for crash interception and analysis.
 * 
 * @details This module exists to provide early detection of process instability
 * through Windows Vectored Exception Handling. Unlike traditional SEH, VEH allows
 * us to intercept exceptions before the standard handler chain, enabling custom
 * recovery logic and forensic analysis.
 * 
 * The design prioritizes:
 * - Low overhead: VEH registration is a one-time cost
 * - Early detection: First responder to exceptions
 * - Context preservation: Full CPU state capture
 * 
 * @security This handler has access to the complete process state. Care must be
 * taken to prevent information disclosure through logging or error messages.
 * 
 * @performance The exception handler is called on every exception, including those
 * that are expected (e.g., CLR exceptions). The handler must be extremely efficient
 * to avoid impacting process performance.
 */
```

**Required Documentation Sections**:
* `@file`: File purpose and high-level overview
* `@brief`: One-line summary
* `@details`: Comprehensive explanation of *why* the component exists
* `@security`: Security implications and considerations
* `@performance`: Performance characteristics and trade-offs
* `@param`: Parameter descriptions (for functions)
* `@return`: Return value description (for functions)
* `@throws`: Exception specifications (if applicable)
* `@see`: Related components and references

**Documentation Standards**:
* Use complete sentences with proper grammar
* Explain the *why*, not just the *what*
* Include usage examples for complex APIs
* Document invariants and preconditions
* Reference Windows API documentation where applicable
* Update documentation in sync with code changes

**Documentation Generation**:
* Doxygen configuration file: `Doxyfile`
* Output format: HTML and LaTeX
* Generated documentation location: `docs/generated/`
* Diagrams: Class diagrams and call graphs via GraphViz integration

---

## Summary

The Sentinel System Monitor represents a high-assurance approach to Windows process monitoring, combining:

* **Native Windows APIs** for deep system integration
* **Advanced exception handling** for crash interception
* **Handle enumeration** for security auditing
* **Virtualized integrity checking** for tamper resistance
* **Secure communication** for reliable telemetry

Built with modern C++20 and MSVC, targeting Windows x64 exclusively, Sentinel provides enterprise-grade monitoring capabilities while maintaining the highest standards of code quality and documentation.

Each module is designed with security, performance, and maintainability in mind, following the philosophy of "Native Windows Reliability & Introspection."
