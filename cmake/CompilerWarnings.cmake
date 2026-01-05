# Compiler warnings configuration for MSVC
# Enforces strict warning levels and treats warnings as errors

if(MSVC)
    # Add compiler flags to all targets
    add_compile_options(
        /W4              # Warning level 4 (highest standard warning level)
        /WX              # Treat warnings as errors
        /permissive-     # Enforce standards conformance
        /Zc:__cplusplus  # Enable updated __cplusplus macro
    )
    
    message(STATUS "MSVC compiler warnings configured: /W4 /WX /permissive- /Zc:__cplusplus")
else()
    message(WARNING "Non-MSVC compiler detected. This project is designed for MSVC on Windows x64.")
endif()
