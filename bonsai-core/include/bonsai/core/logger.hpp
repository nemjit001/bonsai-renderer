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

/// @brief Logger singleton for easily accessible logging through spdlog.
class Logger
{
private:
    Logger() = default;
public:
    ~Logger() = default;

    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;

    /// @brief Access the logger singleton.
    static Logger* get();

    /// @brief Set the minimum log level to report.
    /// @param level Minimum log level to report.
    void set_min_log_level(LogLevel level);

    template<typename... Args>
    static void trace(Args&&... args) { spdlog::trace(std::forward<Args>(args)...); }

    template<typename... Args>
    void debug(Args&&... args) { spdlog::debug(std::forward<Args>(args)...); }

    template<typename... Args>
    void info(Args&&... args) { spdlog::info(std::forward<Args>(args)...); }

    template<typename... Args>
    void warn(Args&&... args) { spdlog::warn(std::forward<Args>(args)...); }

    template<typename... Args>
    void error(Args&&... args) { spdlog::error(std::forward<Args>(args)...); }

    template<typename... Args>
    void critical(Args&&... args) { spdlog::critical(std::forward<Args>(args)...); }
};

#define BONSAI_ENGINE_LOG_TRACE(...)       (Logger::get()->trace(__VA_ARGS__))
#define BONSAI_ENGINE_LOG_DEBUG(...)       (Logger::get()->debug(__VA_ARGS__))
#define BONSAI_ENGINE_LOG_INFO(...)        (Logger::get()->info(__VA_ARGS__))
#define BONSAI_ENGINE_LOG_WARN(...)        (Logger::get()->warn(__VA_ARGS__))
#define BONSAI_ENGINE_LOG_ERROR(...)       (Logger::get()->error(__VA_ARGS__))
#define BONSAI_ENGINE_LOG_CRITICAL(...)    (Logger::get()->critical(__VA_ARGS__))

#endif //BONSAI_RENDERER_LOGGER_HPP