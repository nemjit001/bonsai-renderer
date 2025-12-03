#pragma once
#ifndef BONSAI_RENDERER_RENDERER_HPP
#define BONSAI_RENDERER_RENDERER_HPP

#include "bonsai/render_backend/render_backend.hpp"

class Renderer
{
public:
    explicit Renderer(RenderBackend* render_backend);
    ~Renderer() = default;

    Renderer(Renderer const&) = default;
    Renderer& operator=(Renderer const&) = default;

    /// @brief Handle a surface resize event.
    /// @param width New surface width in pixels.
    /// @param height New surface height in pixels.
    void on_resize(uint32_t width, uint32_t height);

    /// @brief Draw a new frame using the renderer.
    void render();

private:
    RenderBackend* m_render_backend = nullptr;
};

#endif //BONSAI_RENDERER_RENDERER_HPP