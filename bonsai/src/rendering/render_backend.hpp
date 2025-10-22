#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_HPP

#include <cstdint>

class Surface;
struct FrameState;

struct RenderBackendImpl;
class RenderBackend
{
public:
    explicit RenderBackend(Surface const* surface);
    ~RenderBackend();

    RenderBackend(RenderBackend const&) = delete;
    RenderBackend& operator=(RenderBackend const&) = delete;

    /// @brief Handle a window resize in the render backend. Recreates swap chain state.
    /// @param width
    /// @param height
    void on_resize(uint32_t width, uint32_t height);

    /// @brief Start a new frame in the render backend.
    /// @return The frame state for this frame, or nullptr on failure.
    [[nodiscard]] FrameState const* start_frame();

    /// @brief Present the currently acquired frame.
    /// @param frame_state Last acquired frame.
    void present(FrameState const* frame_state);

    /// @brief Get the raw backend implementation handle.
    /// @return An opaque pointer to the backend implementation.
    [[nodiscard]] RenderBackendImpl const* get_raw() const { return m_impl; }

private:
    RenderBackendImpl* m_impl = nullptr;
};

#endif //BONSAI_RENDERER_RENDER_BACKEND_HPP