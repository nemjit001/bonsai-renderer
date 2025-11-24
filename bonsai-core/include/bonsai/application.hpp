#pragma once
#ifndef BONSAI_RENDERER_APPLICATION_HPP
#define BONSAI_RENDERER_APPLICATION_HPP

#include "core/dylib_loader.hpp"
#include "bonsai_export.hpp"
#include "engine_api.hpp"

class Application;
typedef Application*(BONSAI_APICALL *PFN_CreateApplication)(EngineAPI*);
typedef void*(BONSAI_APICALL *PFN_DestroyApplication)(Application const*);

/// @brief Application module handle containing module handling functions.
struct ApplicationModule
{
    DylibHandle const* library;
    PFN_CreateApplication create_application;
    PFN_DestroyApplication destroy_application;
};

/// @brief Load an application module library.
/// @param module_name Module name to load.
/// @return A new application module.
ApplicationModule load_application_module(char const* module_name);

/// @brief Unload the application module from memory.
/// @param app_module Application module.
void unload_application_module(ApplicationModule const& app_module);

/// @brief The bonsai Application class can be specialized by application modules.
class BONSAI_API Application
{
public:
    explicit Application(EngineAPI* engine_api);
    virtual ~Application() = default;

    /// @brief Update the application state.
    /// @param delta Time delta between frame updates.
    virtual void update(double delta);

    /// @brief Get the application name.
    /// @return
    virtual char const* name() const;
};

#endif //BONSAI_RENDERER_APPLICATION_HPP