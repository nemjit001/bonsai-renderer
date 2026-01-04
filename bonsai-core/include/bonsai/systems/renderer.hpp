#pragma once
#ifndef BONSAI_RENDERER_RENDERER_HPP
#define BONSAI_RENDERER_RENDERER_HPP

#include "bonsai/render_backend/render_backend.hpp"

class Renderer
{
public:
    explicit Renderer(RenderBackend* render_backend);
    ~Renderer();

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;

    /// @brief Handle a surface resize event.
    /// @param width New surface width in pixels.
    /// @param height New surface height in pixels.
    void on_resize(uint32_t width, uint32_t height);

    /// @brief Draw a new frame using the renderer.
    void render();

private:
    RenderBackend* m_render_backend = nullptr;
    RenderExtent2D m_swap_extent = {};
    ShaderPipeline* m_shader_pipeline = nullptr;
    RenderBuffer* m_vertex_buffer = nullptr;
    RenderBuffer* m_index_buffer = nullptr;
};

#endif //BONSAI_RENDERER_RENDERER_HPP