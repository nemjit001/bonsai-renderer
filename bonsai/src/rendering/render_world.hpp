#pragma once
#ifndef BONSAI_RENDERER_RENDER_WORLD_HPP
#define BONSAI_RENDERER_RENDER_WORLD_HPP

class World;

class RenderWorld
{
public:
    /// @brief Sync the render world with the host-side world representation.
    /// @param world World to sync with.
    void sync(World const& world);

private:
    struct Impl;
    Impl* m_impl = nullptr;
};

#endif //BONSAI_RENDERER_RENDER_WORLD_HPP