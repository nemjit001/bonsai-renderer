#pragma once
#ifndef BONSAI_RENDERER_LOGGER_HPP
#define BONSAI_RENDERER_LOGGER_HPP

#include <spdlog/spdlog.h>

/// @brief Available levels for logging.
enum class LogLevel
{
    Trace       = SPDLOG_LEVEL_TRACE,
    Debug       = SPDLOG_LEVEL_DEBUG,
    Info        = SPDLOG_LEVEL_INFO,
    Warn        = SPDLOG_LEVEL_WARN,
    Error       = SPDLOG_LEVEL_ERROR,
    Critical    = SPDLOG_LEVEL_CRITICAL,
    Off         = SPDLOG_LEVEL_OFF,
};

/// @brief Static logger class.
class Logger
{
public:
    /// @brief Set the minimum log level to report.
    /// @param level Minimum log level to report.
    static void set_min_log_level(LogLevel level);

    template<typename... Args>
    static void trace(Args&&... args) { spdlog::trace(std::forward<Args>(args)...); }

    template<typename... Args>
    static void debug(Args&&... args) { spdlog::debug(std::forward<Args>(args)...); }

    template<typename... Args>
    static void info(Args&&... args) { spdlog::info(std::forward<Args>(args)...); }

    template<typename... Args>
    static void warn(Args&&... args) { spdlog::warn(std::forward<Args>(args)...); }

    template<typename... Args>
    static void error(Args&&... args) { spdlog::error(std::forward<Args>(args)...); }

    template<typename... Args>
    static void critical(Args&&... args) { spdlog::critical(std::forward<Args>(args)...); }
};

#define BONSAI_LOG_TRACE(...)       (Logger::trace(__VA_ARGS__))
#define BONSAI_LOG_DEBUG(...)       (Logger::debug(__VA_ARGS__))
#define BONSAI_LOG_INFO(...)        (Logger::info(__VA_ARGS__))
#define BONSAI_LOG_WARN(...)        (Logger::warn(__VA_ARGS__))
#define BONSAI_LOG_ERROR(...)       (Logger::error(__VA_ARGS__))
#define BONSAI_LOG_CRITICAL(...)    (Logger::critical(__VA_ARGS__))

#endif //BONSAI_RENDERER_LOGGER_HPP