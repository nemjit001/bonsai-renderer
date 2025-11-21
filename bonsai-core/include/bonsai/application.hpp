#pragma once
#ifndef BONSAI_RENDERER_APPLICATION_HPP
#define BONSAI_RENDERER_APPLICATION_HPP

#include "bonsai_api.hpp"

/// @brief The bonsai Application class can be specialized by application modules.
class BONSAI_API Application
{
public:
    Application();
    virtual ~Application();

    /// @brief Update the application state.
    /// @param delta Time delta between frame updates.
    virtual void update(double delta);
};

typedef Application*(BONSAI_APICALL *PFN_CreateApplication)();
typedef void*(BONSAI_APICALL *PFN_DestroyApplication)(Application*);

extern "C" BONSAI_API Application* BONSAI_APICALL create_application();
extern "C" BONSAI_API void BONSAI_APICALL destroy_application(Application* app);

#endif //BONSAI_RENDERER_APPLICATION_HPP