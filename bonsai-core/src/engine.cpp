#include "bonsai/engine.hpp"

#include <cstdio>

#include "bonsai/core/dylib_loader.hpp"
#include "bonsai/application.hpp"

Engine::Engine()
{
    printf("Initialized Engine\n");
}

Engine::~Engine()
{
    printf("Shutting down Engine\n");
}

void Engine::run(char const* app_name)
{
    // This loads an application library dynamically
    DylibHandle const* app_library = bonsai_load_library(bonsai_lib_name(app_name));
    PFN_CreateApplication const create_application = reinterpret_cast<PFN_CreateApplication>(bonsai_get_proc_address(app_library, "create_application"));
    PFN_DestroyApplication const destroy_application = reinterpret_cast<PFN_DestroyApplication>(bonsai_get_proc_address(app_library, "destroy_application"));
    if (create_application == nullptr || destroy_application == nullptr)
    {
        printf("Failed to load Bonsai application symbols from %s\n", bonsai_lib_name(app_name).c_str());
        return;
    }

    // This actually creates the app
    Application* app = create_application();
    printf("Initialized  Bonsai application: %s\n", app->name());

    destroy_application(app);
    bonsai_unload_library(app_library);
}

