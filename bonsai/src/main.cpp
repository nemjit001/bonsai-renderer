#include "logger.hpp"
#include "platform.hpp"

int main()
{
    Logger& logger = Logger::get();
    logger.set_min_log_level(LogLevel::Trace);
    BONSAI_LOG_INFO("Initialized Logger");

    Platform& platform = Platform::get();
    BONSAI_LOG_INFO("Initialized Platform");

    SurfaceConfig const surface_config{ true /* resizable */, true /* allow_high_dpi */ };
    Surface* surface = platform.create_surface("Bonsai Renderer", 1280, 720, surface_config);
    BONSAI_LOG_INFO("Initialized application surface");

    // FIXME(nemjit001): This is kinda ugly since the platform message loop should dictate application lifetime?
    bool running = true;
    while (running)
    {
        platform.pump_messages();
        running = false;
    }

    platform.destroy_surface(surface);
    return 0;
}
