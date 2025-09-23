#include "logger.hpp"

int main() {
    Logger& Logger = Logger::get();
    Logger.setMinLogLevel(LogLevel::Trace);
    BONSAI_LOG_INFO("Initialized Bonsai!");
}
