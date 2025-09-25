#include "core/logger.hpp"
#include "platform/platform.hpp"

int main()
{
    // Initialize global logger
    Logger& logger = Logger::get();
    logger.set_min_log_level(LogLevel::Trace);
    BONSAI_LOG_INFO("Initialized Logger");

    // Initialize platform system
    Platform platform;
    BONSAI_LOG_INFO("Initialized Platform");

    // Create a platform surface
    SurfaceConfig const surface_config{ true /* resizable */, true /* allow_high_dpi */ };
    Surface* surface = platform.create_surface("Bonsai Renderer", 1600, 900, surface_config);
    BONSAI_LOG_INFO("Initialized application surface");

    // Hook up application quit handler
    // TODO(nemjit001): This should be managed in an engine/application class
    bool running = true;
    platform.set_platform_user_data(&running);
    platform.set_platform_quit_callback([](void* user_data)
    {
        bool* running_ = static_cast<bool*>(user_data);
        *running_ = false;
    });
    BONSAI_LOG_INFO("Initialized Bonsai!");

    // Enter main loop
    while (running)
    {
        platform.pump_messages();
    }

    // Do some cleanup
    platform.destroy_surface(surface);
    return 0;
}
