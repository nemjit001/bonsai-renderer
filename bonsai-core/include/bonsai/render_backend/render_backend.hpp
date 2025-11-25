#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_HPP

#include "bonsai/core/platform.hpp"

class RenderBackend
{
protected:
    RenderBackend() = default;
public:
    virtual ~RenderBackend() = default;

    static RenderBackend* create(PlatformSurface* platform_surface);
};

#endif //BONSAI_RENDERER_RENDER_BACKEND_HPP