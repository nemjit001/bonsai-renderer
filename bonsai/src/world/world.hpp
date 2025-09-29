#pragma once
#ifndef BONSAI_RENDERER_WORLD_HPP
#define BONSAI_RENDERER_WORLD_HPP

#include <string>
#include "entity.hpp"

/// @brief World, represents a collection of entities that together form a scene.
class World
{
public:
    [[nodiscard]] Entity*       get_root()          { return &m_root; }
    [[nodiscard]] Entity const* get_root() const    { return &m_root; }

private:
    Entity m_root{ "Root" };
};

#endif //BONSAI_RENDERER_WORLD_HPP