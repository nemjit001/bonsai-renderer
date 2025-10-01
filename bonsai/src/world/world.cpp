#include "world.hpp"

void World::update(double delta)
{
    std::vector<Entity::Ref> stack;
    stack.reserve(256); // Reserve space for processing at least 256 nodes to avoid unnecessary allocations
    stack.push_back(m_root);
    while (!stack.empty())
    {
        // Fetch next node from the stack
        Entity::Ref const current = stack.back();
        stack.pop_back();

        // Get entity data before update, copies are used to avoid iterator invalidation during update in entities.
        std::vector<Entity::ComponentRef> components = current->get_components();
        std::vector<Entity::Ref> children = current->get_children();

        // Update all entity components
        for (auto const& component : components)
        {
            component->update(delta);
        }

        // Push entity children onto the stack for further processing
        for (auto const& child : children)
        {
            stack.push_back(child);
        }
    }
}
