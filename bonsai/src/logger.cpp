#include "logger.hpp"

#include <spdlog/sinks/basic_file_sink.h>

Logger::Logger()
{
    //
}

Logger& Logger::get()
{
    static Logger instance;
    return instance;
}

void Logger::setMinLogLevel(LogLevel const& level)
{
    switch (level)
    {
    case LogLevel::Trace:
        spdlog::set_level(spdlog::level::trace);
        break;
    case LogLevel::Debug:
        spdlog::set_level(spdlog::level::debug);
        break;
    case LogLevel::Info:
        spdlog::set_level(spdlog::level::info);
        break;
    case LogLevel::Warning:
        spdlog::set_level(spdlog::level::warn);
        break;
    case LogLevel::Error:
        spdlog::set_level(spdlog::level::err);
        break;
    case LogLevel::Critical:
        spdlog::set_level(spdlog::level::critical);
        break;
    case LogLevel::None:
        spdlog::set_level(spdlog::level::off);
        break;
    default:
        break;
    }
}
