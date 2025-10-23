#include "rendering/render_world.hpp"

#include <vector>
#include "world/world.hpp"
#include "components/render_component.hpp"
#include "render_backend_vulkan.hpp"

RenderWorld::RenderWorld(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    //
}

void RenderWorld::sync(World const& world)
{
    Entity::Ref const root = world.get_root();
    std::vector<Entity::Ref> stack;
    stack.reserve(256);
    stack.push_back(root);
    while (!stack.empty())
    {
        // Fetch next node from the stack
        Entity::Ref const current = stack.back();
        stack.pop_back();

        if (current->has_component<RenderComponent>())
        {
            ComponentHandle<RenderComponent> const rc = current->get_component<RenderComponent>();
            AssetHandle<Model> const model = rc->get_model();

            // TODO(nemjit001): Check if model has an allocation in the model cache
        }

        // Submit children for sync
        for (auto const& child : current->get_children())
        {
            stack.push_back(child);
        }
    }
}
