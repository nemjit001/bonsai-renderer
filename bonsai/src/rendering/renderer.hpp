#pragma once
#ifndef BONSAI_RENDERER_RENDERER_HPP
#define BONSAI_RENDERER_RENDERER_HPP

/// @brief Renderer, exposes a rendering API abstraction over a low level graphics API.
class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;
};

#endif //BONSAI_RENDERER_RENDERER_HPP