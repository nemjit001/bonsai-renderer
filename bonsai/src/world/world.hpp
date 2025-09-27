#pragma once
#ifndef BONSAI_RENDERER_WORLD_HPP
#define BONSAI_RENDERER_WORLD_HPP

#include <string>
#include <memory>
#include <vector>

/// @brief Base entity class, represents anything that can be stored in the world.
class Entity
{
public:
    using Ref = std::shared_ptr<Entity>;

    /// @brief Create a new entity or derived entity.
    /// @tparam EntityType Entity type to create.
    /// @tparam Args Entity type constructor arguments.
    /// @param args Constructor arguments.
    /// @return A new Entity::Ref representing the newly created entity.
    template <typename EntityType, typename... Args>
    static Ref create(Args&&... args)
    {
        static_assert(std::is_base_of_v<Entity, EntityType> && "EntityType should derive from Entity!");
        return Ref(new EntityType(std::forward<Args>(args)...));
    }

    explicit Entity(std::string const& name);
    virtual ~Entity() = default;

    Entity(Entity const&) = default;
    Entity& operator=(Entity const&) = default;

    /// @brief Add a child to this entity.
    /// @param entity Entity to add as child.
    void add_child(Ref entity);

private:
    std::string         m_name;
    std::vector<Ref>    m_children;
};

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