#include "logger.hpp"

#ifdef GODOT_EXTENSION
#include <godot_cpp/variant/utility_functions.hpp>
#endif

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <string>

namespace godot_ffmpeg {

#ifdef GODOT_EXTENSION

// Godot implementation
void log_internal(LogLevel level, const char* format, ...) {
    // Format the message
    va_list args;
    va_start(args, format);
    
    // Calculate required buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        return;
    }
    
    // Allocate buffer and format
    std::string message(size + 1, '\0');
    vsnprintf(message.data(), size + 1, format, args);
    va_end(args);
    
    // Remove null terminator (std::string handles it)
    message.resize(size);
    
    // Route to appropriate Godot function
    // Concatenate prefix and message for single-argument functions
    std::string full_message;
    switch (level) {
        case LogLevel::DEBUG:
            full_message = "[DEBUG] " + message;
            godot::UtilityFunctions::print(full_message.c_str());
            break;
        case LogLevel::INFO:
            full_message = "[INFO] " + message;
            godot::UtilityFunctions::print(full_message.c_str());
            break;
        case LogLevel::WARN:
            full_message = "[WARN] " + message;
            godot::UtilityFunctions::print(full_message.c_str());
            break;
        case LogLevel::ERROR:
            full_message = "[ERROR] " + message;
            godot::UtilityFunctions::printerr(full_message.c_str());
            break;
    }
}

#else

// Terminal/headless implementation with ANSI color codes
namespace {
    constexpr const char* ANSI_RESET = "\033[0m";
    constexpr const char* ANSI_GREEN = "\033[32m";
    constexpr const char* ANSI_YELLOW = "\033[33m";
    constexpr const char* ANSI_RED = "\033[31m";
    constexpr const char* ANSI_CYAN = "\033[36m";
}

void log_internal(LogLevel level, const char* format, ...) {
    // Format the message
    va_list args;
    va_start(args, format);
    
    // Calculate required buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        return;
    }
    
    // Allocate buffer and format
    std::string message(size + 1, '\0');
    vsnprintf(message.data(), size + 1, format, args);
    va_end(args);
    
    // Remove null terminator
    message.resize(size);
    
    // Output with appropriate color and prefix
    switch (level) {
        case LogLevel::DEBUG:
            std::cout << ANSI_CYAN << "[DEBUG] " << ANSI_RESET << message << std::endl;
            break;
        case LogLevel::INFO:
            std::cout << ANSI_GREEN << "[INFO] " << ANSI_RESET << message << std::endl;
            break;
        case LogLevel::WARN:
            std::cout << ANSI_YELLOW << "[WARN] " << ANSI_RESET << message << std::endl;
            break;
        case LogLevel::ERROR:
            std::cout << ANSI_RED << "[ERROR] " << ANSI_RESET << message << std::endl;
            break;
    }
}

#endif // GODOT_EXTENSION

} // namespace godot_ffmpeg
