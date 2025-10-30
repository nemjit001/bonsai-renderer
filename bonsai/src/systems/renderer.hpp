#pragma once
#ifndef BONSAI_RENDERER_RENDERER_HPP
#define BONSAI_RENDERER_RENDERER_HPP

#include "platform/platform.hpp"
#include "rhi/rhi.hpp"

class Renderer
{
public:
    explicit Renderer(Surface* surface);
    ~Renderer();

    Renderer(Renderer const&) = default;
    Renderer& operator=(Renderer const&) = default;

    /// @brief Handle a window resize event in the renderer.
    /// @param width
    /// @param height
    void on_resize(uint32_t width, uint32_t height);

    /// @brief Render the current world state.
    void render();

    /// @brief Get the render device used by the renderer.
    /// @return The render device handle used by the renderer.
    RenderDeviceHandle get_render_device() const { return m_render_device; }

private:
    RHIInstanceHandle       m_rhi_instance;
    RenderDeviceHandle      m_render_device;
    SwapChainHandle         m_swap_chain;
    CommandAllocatorHandle  m_command_allocator;
    CommandBufferHandle     m_frame_commands;
};

#endif //BONSAI_RENDERER_RENDERER_HPP