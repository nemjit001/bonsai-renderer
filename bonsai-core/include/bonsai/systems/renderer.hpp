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

    void render();

private:
    RenderBackend* m_render_backend = nullptr;
};

#endif //BONSAI_RENDERER_RENDERER_HPP