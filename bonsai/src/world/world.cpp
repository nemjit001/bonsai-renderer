#include "world.hpp"

void World::update(double delta)
{
    std::vector<Entity::Ref> stack;
    stack.reserve(256); // Reserve space for processing at least 256 nodes to avoid unnecessary allocations
    stack.push_back(m_root);
    while (!stack.empty())
    {
        // Fetch next node from the stack & update components
        Entity::Ref const current = stack.back();
        stack.pop_back();
        for (auto const& component : current->get_components())
        {
            component->update(delta);
        }

        // Push children onto the stack for further processing
        for (auto const& child : current->get_children())
        {
            stack.push_back(child);
        }
    }
}
