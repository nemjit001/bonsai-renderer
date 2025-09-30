#pragma once
#ifndef BONSAI_RENDERER_WORLD_HPP
#define BONSAI_RENDERER_WORLD_HPP

#include "entity.hpp"

/// @brief World, represents a collection of entities that together form a scene.
class World
{
public:
    /// @brief Update the world state, updating all entities in the world in the process.
    /// @param delta Time delta between this and last update in milliseconds.
    void update(double delta);

    [[nodiscard]] Entity::Ref           get_root()          { return m_root; }
    [[nodiscard]] Entity::Ref const&    get_root() const    { return m_root; }

private:
    Entity::Ref m_root = Entity::create<Entity>("Root");
};

#endif //BONSAI_RENDERER_WORLD_HPP