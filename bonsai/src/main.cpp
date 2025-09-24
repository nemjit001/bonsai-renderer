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
    platform.set_platform_surface_resize_callback([]([[maybe_unused]] void* user_data, uint32_t width, uint32_t height)
    {
        BONSAI_LOG_TRACE("Surface resized ({}x{})", width, height);
    });
    platform.set_platform_surface_closed_callback([]([[maybe_unused]] void* user_data)
    {
        BONSAI_LOG_TRACE("Surface closed");
    });
    BONSAI_LOG_INFO("Initialized application surface");

    // FIXME(nemjit001): This is kinda ugly since the platform message loop should dictate application lifetime?
    bool running = true;
    platform.set_platform_user_data(&running);
    platform.set_platform_quit_callback([](void* user_data)
    {
        bool* running_ = static_cast<bool*>(user_data);
        *running_ = false;
    });
    BONSAI_LOG_INFO("Initialized Bonsai!");

    while (running)
    {
        platform.pump_messages();
    }

    platform.destroy_surface(surface);
    return 0;
}
