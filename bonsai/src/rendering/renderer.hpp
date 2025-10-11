#pragma once
#ifndef BONSAI_RENDERER_RENDERER_HPP
#define BONSAI_RENDERER_RENDERER_HPP

#include <cstdint>

class Surface;
class World;

/// @brief Renderer, implements the complete render pipeline for Bonsai.
class Renderer
{
public:
    explicit Renderer(Surface const* surface);
    ~Renderer();

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;

    /// @brief Handle a surface resize event in the renderer.
    /// @param width New surface width in pixels.
    /// @param height New surface height in pixels.
    void on_resize(uint32_t width, uint32_t height);

    /// @brief Render a World using the renderer.
    /// @param render_world World to use for rendering.
    /// @param delta Time delta between rendered frames in milliseconds.
    void render(World const& render_world, double delta);

private:
    struct Impl;
    Impl* m_impl = nullptr;
};

#endif //BONSAI_RENDERER_RENDERER_HPP