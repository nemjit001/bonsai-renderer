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

    /// @brief Set this entity's name, will make the name unique within the parent node's children.
    /// @param name New name of this entity.
    void set_name(std::string const& name);

    /// @brief Get this entity's name.
    /// @return The entity name.
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

    /// @brief Get a child of this node by name.
    /// @param name Child node name to get.
    /// @return The child node Entity::Ref, or an empty ref if the child does not exist.
    [[nodiscard]] Ref get_child(std::string const& name);

    /// @brief Get the list of children of this node.
    /// @return A vector of child nodes.
    [[nodiscard]] std::vector<Ref> const& children() const { return m_children; }

    /// @brief Get the world-space affine transformation matrix.
    [[nodiscard]] glm::mat4 get_world_space_transform() const;

    /// @brief Set the local entity transform.
    /// @param transform New entity transform.
    void set_transform(Transform const& transform) { m_transform = transform; }

    /// @brief Get the local entity transform.
    [[nodiscard]] Transform get_transform() const { return m_transform; }

    /// @brief Update the state of this entity and its children.
    /// @param delta Time delta between updates in milliseconds.
    void update_tree(double delta);

protected:
    /// @brief Update the state of this entity.
    /// @param delta Time delta between updates in milliseconds.
    virtual void update(double delta);

private:
    /// @brief Find a unique name for this entity in the parent entity.
    /// @param parent Parent node to use for unique name search, may be nullptr.
    /// @param name Candidate name to make unique.
    /// @return A guaranteed unique name in the given parent node.
    [[nodiscard]] static std::string get_unique_name_in_parent(Entity const* parent, std::string const& name);

private:
    std::string         m_name;
    Entity*             m_parent;
    std::vector<Ref>    m_children;
    Transform           m_transform;
};

#endif //BONSAI_RENDERER_ENTITY_HPP