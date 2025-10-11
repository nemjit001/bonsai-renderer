#pragma once
#ifndef BONSAI_RENDERER_LOGGER_HPP
#define BONSAI_RENDERER_LOGGER_HPP

#include <spdlog/spdlog.h>

/// @brief LogLevels that may be used to filter logs.
enum class LogLevel : uint8_t
{
    Trace       = 0,
    Debug       = 1,
    Info        = 2,
    Warning     = 3,
    Error       = 4,
    Critical    = 5,
    None        = 6,
    NUM_LEVELS,
};

/// @brief Logger singleton wrapper. Handles application-wide logging tasks.
class Logger
{
public:
    /// @brief Get the logger singleton instance.
    static Logger& get();

    ~Logger() = default;
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;

    /// @brief Set the minimum log level for the logger to use.
    /// @param level Minimum log level to show.
    void set_min_log_level(LogLevel const& level);

    template <typename... Args>
    void trace(Args&&... args);

    template <typename... Args>
    void debug(Args&&... args);

    template <typename... Args>
    void info(Args&&... args);

    template <typename... Args>
    void warning(Args&&... args);

    template <typename... Args>
    void error(Args&&... args);

    template <typename... Args>
    void critical(Args&&... args);

private:
    Logger() = default;
};

#define BONSAI_LOG_TRACE(...)       (Logger::get().trace(__VA_ARGS__))
#define BONSAI_LOG_DEBUG(...)       (Logger::get().debug(__VA_ARGS__))
#define BONSAI_LOG_INFO(...)        (Logger::get().info(__VA_ARGS__))
#define BONSAI_LOG_WARNING(...)     (Logger::get().warning(__VA_ARGS__))
#define BONSAI_LOG_ERROR(...)       (Logger::get().error(__VA_ARGS__))
#define BONSAI_LOG_CRITICAL(...)    (Logger::get().critical(__VA_ARGS__))

#pragma region implementation

template <typename... Args>
void Logger::trace(Args&&... args)
{
    spdlog::trace(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::debug(Args&&... args)
{
    spdlog::debug(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::info(Args&&... args)
{
    spdlog::info(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::warning(Args&&... args)
{
    spdlog::warn(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::error(Args&&... args)
{
    spdlog::error(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::critical(Args&&... args)
{
    spdlog::critical(std::forward<Args>(args)...);
}

#pragma endregion

#endif //BONSAI_RENDERER_LOGGER_HPP