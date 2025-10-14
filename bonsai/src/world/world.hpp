#pragma once
#ifndef BONSAI_RENDERER_WORLD_HPP
#define BONSAI_RENDERER_WORLD_HPP

#include <filesystem>
#include <string>
#include "assets/asset.hpp"
#include "entity.hpp"

/// @brief World, represents a collection of entities that together form a scene.
class World : public Asset
{
public:
    /// @brief Update the world state, updating all entities in the world in the process.
    /// @param delta Time delta between this and last update in milliseconds.
    void update(double delta);

    /// @brief Set the world's internal name.
    /// @param name
    void set_name(std::string const& name) { m_name = name; }

    /// @brief Get the world's name.
    /// @return The world name.
    [[nodiscard]] std::string get_name() const { return m_name; }

    [[nodiscard]] Entity::Ref           get_root()          { return m_root; }
    [[nodiscard]] Entity::Ref const&    get_root() const    { return m_root; }

private:
    std::string m_name = "Nameless World";
    Entity::Ref m_root = Entity::create<Entity>("Root");
};

#endif //BONSAI_RENDERER_WORLD_HPP