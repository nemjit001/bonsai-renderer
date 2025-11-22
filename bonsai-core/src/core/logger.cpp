#include "bonsai/core/logger.hpp"

void Logger::set_min_log_level(LogLevel level)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
}
