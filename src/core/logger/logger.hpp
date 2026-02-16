#pragma once

namespace godot_ffmpeg {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

// Internal logging function (implemented in logger.cpp)
void log_internal(LogLevel level, const char* format, ...);

// Macros for easy logging
#define LOG_DEBUG(...) ::godot_ffmpeg::log_internal(::godot_ffmpeg::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) ::godot_ffmpeg::log_internal(::godot_ffmpeg::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...) ::godot_ffmpeg::log_internal(::godot_ffmpeg::LogLevel::WARN, __VA_ARGS__)
#define LOG_ERR(...) ::godot_ffmpeg::log_internal(::godot_ffmpeg::LogLevel::ERROR, __VA_ARGS__)

} // namespace godot_ffmpeg
