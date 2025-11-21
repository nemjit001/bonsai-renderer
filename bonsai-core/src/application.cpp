#include "bonsai/application.hpp"

ApplicationModule load_application_module(char const* module_name)
{
    DylibHandle const* app_library = bonsai_load_library(bonsai_lib_name(module_name));
    return {
        app_library,
        reinterpret_cast<PFN_CreateApplication>(bonsai_get_proc_address(app_library, "create_application")),
        reinterpret_cast<PFN_DestroyApplication>(bonsai_get_proc_address(app_library, "destroy_application")),
    };
}

void unload_application_module(ApplicationModule const& app_module)
{
    bonsai_unload_library(app_module.library);
}

Application::Application() = default;
Application::~Application() = default;

void Application::update([[maybe_unused]] double delta)
{
    //
}
