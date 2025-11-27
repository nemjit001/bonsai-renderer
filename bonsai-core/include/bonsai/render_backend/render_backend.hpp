#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_HPP

#include "bonsai/core/platform.hpp"

/// @brief Render backend new frame or present result indicating frame state.
enum class RenderBackendFrameResult
{
    Ok = 0,
    SwapOutOfDate,
    FatalError,
};

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

    /// @brief Wait for the backend render device to be idle.
    virtual void wait_idle() const = 0;

    /// @brief Start a new render backend frame.
    /// @return A render backend frame result.
    virtual RenderBackendFrameResult new_frame() = 0;

    /// @brief End the currently active render backend frame.
    /// @return A render backend frame result.
    virtual RenderBackendFrameResult end_frame() = 0;
};

#endif //BONSAI_RENDERER_RENDER_BACKEND_HPP