#include "logger.hpp"
#include "platform.hpp"

int main()
{
    Logger& logger = Logger::get();
    logger.set_min_log_level(LogLevel::Trace);
    BONSAI_LOG_INFO("Initialized Logger");

    Platform& platform = Platform::get();
    BONSAI_LOG_INFO("Initialized Platform");

    bool running = false;
    while (running)
    {
        platform.pump_messages();
    }

    return 0;
}
