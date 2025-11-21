#include "bonsai/engine.hpp"

#include "bonsai/application.hpp"

Engine::Engine()
{
    //
}

Engine::~Engine()
{
    //
}

void Engine::run(char const* app_name)
{
    ApplicationModule const app_module = load_application_module(app_name);
    Application* app = app_module.create_application();

    // Enter the engine main loop
    bool running = false;
    while (running)
    {
        app->update(0.0);
    }

    app_module.destroy_application(app);
    unload_application_module(app_module);
}

