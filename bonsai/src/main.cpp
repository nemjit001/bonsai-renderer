#include "logger.hpp"
#include "platform.hpp"

int main()
{
    // Initialize global logger
    Logger& logger = Logger::get();
    logger.set_min_log_level(LogLevel::Trace);
    BONSAI_LOG_INFO("Initialized Logger");

    // Initialize platform system
    Platform platform;
    BONSAI_LOG_INFO("Initialized Platform");

    // Create a platform surface and hook up surface callback functions
    SurfaceConfig const surface_config{ true /* resizable */, true /* allow_high_dpi */ };
    Surface* surface = platform.create_surface("Bonsai Renderer", 1600, 900, surface_config);
    platform.set_platform_surface_resize_callback([]([[maybe_unused]] void* user_data, uint32_t width, uint32_t height)
    {
        BONSAI_LOG_TRACE("Surface resized ({}x{})", width, height);
    });
    platform.set_platform_surface_closed_callback([]([[maybe_unused]] void* user_data)
    {
        BONSAI_LOG_TRACE("Surface closed");
    });
    BONSAI_LOG_INFO("Initialized application surface");

    // Hook up application quit handler
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

    // Do some cleanup...
    platform.destroy_surface(surface);
    return 0;
}
