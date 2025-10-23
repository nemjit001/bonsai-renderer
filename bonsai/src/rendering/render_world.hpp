#pragma once
#ifndef BONSAI_RENDERER_RENDER_WORLD_HPP
#define BONSAI_RENDERER_RENDER_WORLD_HPP

class GPUBuffer;
class GPUTexture;
class RenderBackend;
class World;

class RenderWorld
{
public:
    explicit RenderWorld(RenderBackend* render_backend);
    ~RenderWorld() = default;

    RenderWorld(RenderWorld const&) = delete;
    RenderWorld& operator=(RenderWorld const&) = delete;

    /// @brief Sync the render world with the host-side world representation.
    /// @param world World to sync with.
    void sync(World const& world);

private:
    RenderBackend* m_render_backend = nullptr;
};

#endif //BONSAI_RENDERER_RENDER_WORLD_HPP