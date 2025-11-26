#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_HPP

#include "bonsai/core/platform.hpp"

/// @brief The RenderBackend wraps a backend graphics API, providing a common interface for the engine to use.
class RenderBackend
{
protected:
    RenderBackend() = default;
public:
    virtual ~RenderBackend() = default;

    /// @brief Create a render backend.
    /// @param platform_surface Main surface to use for rendering, will be used to initialize the render backend.
    /// @return A new render backend, or nullptr if no backend is active.
    static RenderBackend* create(PlatformSurface* platform_surface);
};

#endif //BONSAI_RENDERER_RENDER_BACKEND_HPP