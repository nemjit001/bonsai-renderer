#include "bonsai/engine.hpp"

#include <cstdio>
#include "bonsai/core/platform.hpp"
#include "bonsai/application.hpp"

static Platform* s_platform = nullptr;
static PlatformSurface* s_main_surface = nullptr;

Engine::Engine()
{
    printf("Initializing Platform\n");
    s_platform = new Platform();

    printf("Initializing main surface\n");
    PlatformSurfaceConfig const main_surface_config{ true, true };
    s_main_surface = s_platform->create_surface("Bonsai Application", 1600, 900, main_surface_config);

    s_platform->set_surface_resized_callback([](PlatformSurface* surface, uint32_t width, uint32_t height) {
        printf("Window resized (%d x %d)\n", width, height);
    });
}

Engine::~Engine()
{
    printf("Shutting down...\n");
    s_platform->destroy_surface(s_main_surface);
    delete s_platform;
}

void Engine::run(char const* app_name)
{
    // Load application module
    ApplicationModule const app_module = load_application_module(app_name);
    if (app_module.library == nullptr)
    {
        printf("Failed to load application module: \"%s\"\n", app_name);
        return;
    }

    // Create application
    Application* app = app_module.create_application();

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

