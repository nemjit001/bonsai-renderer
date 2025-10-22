#include "render_world.hpp"

#include "render_backend.hpp"
#include "world/world.hpp"

RenderWorld::RenderWorld(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    //
}

void RenderWorld::sync(World const& world)
{
    //
}
