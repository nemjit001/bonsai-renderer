#pragma once
#ifndef BONSAI_RENDERER_RENDER_WORLD_HPP
#define BONSAI_RENDERER_RENDER_WORLD_HPP

class World;

class RenderWorld
{
public:
    RenderWorld();
    ~RenderWorld();

    RenderWorld(RenderWorld const&) = delete;
    RenderWorld& operator=(RenderWorld const&) = delete;

    /// @brief Sync the render world with the host-side world representation.
    /// @param world World to sync with.
    void sync(World const& world);
};

#endif //BONSAI_RENDERER_RENDER_WORLD_HPP