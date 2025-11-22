#include "bonsai/engine.hpp"

#include "bonsai/core/assert.hpp"
#include "bonsai/core/logger.hpp"
#include "bonsai/core/platform.hpp"
#include "bonsai/application.hpp"

static Platform* s_platform = nullptr;
static PlatformSurface* s_main_surface = nullptr;

Engine::Engine()
{
    Logger::set_min_log_level(LogLevel::Trace);

    BONSAI_LOG_INFO("Initializing Platform");
    s_platform = new Platform();

    BONSAI_LOG_INFO("Initializing main surface");
    PlatformSurfaceConfig const main_surface_config{ true, true };
    s_main_surface = s_platform->create_surface("Bonsai Application", 1600, 900, main_surface_config);

    s_platform->set_surface_resized_callback([](PlatformSurface* surface, uint32_t width, uint32_t height) {
        BONSAI_LOG_TRACE("Window resized ({} x {})", width, height);
    });
}

Engine::~Engine()
{
    BONSAI_LOG_INFO("Shutting down...");
    s_platform->destroy_surface(s_main_surface);
    delete s_platform;
}

void Engine::run(char const* app_name)
{
    // Load application module
    ApplicationModule const app_module = load_application_module(app_name);
    if (app_module.library == nullptr
        || app_module.create_application == nullptr
        || app_module.destroy_application == nullptr)
    {
        BONSAI_LOG_ERROR("Failed to load application module: \"{}\"", app_name);
        return;
    }

    // Create application
    Application* app = app_module.create_application();
    BONSAI_ASSERT(app != nullptr);

    // Enter the engine main loop
    bool running = true;
    while (running)
    {
        if (!s_platform->pump_messages())
        {
            running = false;
        }

        app->update(0.0);
    }

    app_module.destroy_application(app);
    unload_application_module(app_module);
}

