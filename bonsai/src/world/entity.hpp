#pragma once
#ifndef BONSAI_RENDERER_ENTITY_HPP
#define BONSAI_RENDERER_ENTITY_HPP

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/// @brief Entity scene transform.
struct Transform
{
    glm::vec3 position  = glm::vec3(0.0F, 0.0F, 0.0F);
    glm::quat rotation  = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
    glm::vec3 scale     = glm::vec3(1.0F, 1.0F, 1.0F);

    /// @brief Calculate the affine transformation matrix representing this Transform.
    /// @return A 4x4 transformation matrix.
    [[nodiscard]] glm::mat4 matrix() const;
};

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
    Entity(std::string const& name, Transform const& transform);
    virtual ~Entity() = default;

    Entity(Entity const&) = default;
    Entity& operator=(Entity const&) = default;

    /// @brief Get this entity's name.
    [[nodiscard]] std::string get_name() const { return m_name; }

    /// @brief Add a child to this entity.
    /// @param entity Entity to add as child.
    void add_child(Ref entity);

    /// @brief Remove a child from this entity.
    /// @param name Name of the child entity to remove.
    void remove_child(std::string const& name);

    /// @brief Check if this node has a child node with the given name
    /// @param name Named node to search for.
    /// @return A boolean indicating child's existence.
    [[nodiscard]] bool has_child(std::string const& name) const;

    /// @brief Get the worldspace affine transformation matrix.
    [[nodiscard]] glm::mat4 get_worldspace_transform() const;

    /// @brief Set the local entity transform.
    /// @param transform New entity transform.
    void set_transform(Transform const& transform) { m_transform = transform; }

    /// @brief Get the local entity transform.
    [[nodiscard]] Transform get_transform() const { return m_transform; }

private:
    std::string         m_name;
    Entity*             m_parent;
    std::vector<Ref>    m_children;
    Transform           m_transform;
};

#endif //BONSAI_RENDERER_ENTITY_HPP