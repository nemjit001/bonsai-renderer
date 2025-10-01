#pragma once
#ifndef BONSAI_RENDERER_ENTITY_HPP
#define BONSAI_RENDERER_ENTITY_HPP

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Entity;

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

/// @brief Component interface, can be added to an entity in the world to provide behaviour.
class Component
{
public:
    virtual ~Component() = default;

    /// @brief Update this component's state.
    /// @param delta Time delta between updates.
    virtual void update([[maybe_unused]] double delta) {};

    /// @brief Set this Component's associated entity.
    /// @param entity Entity to mark as parent.
    void set_entity(Entity* entity) { m_entity = entity; }

    [[nodiscard]] Entity*       entity()        { return m_entity; }
    [[nodiscard]] Entity const* entity() const  { return m_entity; }

private:
    Entity* m_entity = nullptr;
};

/// @brief Base entity class, represents anything that can be stored in the world.
class Entity
{
public:
    using Ref = std::shared_ptr<Entity>;
    using ComponentRef = std::shared_ptr<Component>;

    /// @brief Create a new entity or derived entity.
    /// @tparam EntityType Entity type to create.
    /// @tparam Args Entity type constructor arguments.
    /// @param args Constructor arguments.
    /// @return A new Entity::Ref representing the newly created entity.
    template <typename EntityType, typename... Args>
    static Ref create(Args&&... args);

    Entity();
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
    [[nodiscard]] std::vector<Ref> const& get_children() const { return m_children; }

    /// @brief Get the world-space affine transformation matrix.
    /// @return The entity transformation matrix.
    [[nodiscard]] glm::mat4 get_world_space_transform() const;

    /// @brief Set the local entity transform.
    /// @param transform New entity transform.
    void set_transform(Transform const& transform) { m_transform = transform; }

    /// @brief Get the local entity transform.
    /// @return The entity transform.
    [[nodiscard]] Transform get_transform() const { return m_transform; }

    /// @brief Add a component to this entity.
    /// @tparam ComponentType Component type to add.
    /// @tparam Args Constructor arguments parameter pack.
    /// @param args Component constructor args.
    template <typename ComponentType, typename... Args>
    void add_component(Args&&... args);

    /// @brief Remove a component from this entity by index.
    /// @param index Index of the component to remove.
    void remove_component_by_index(size_t index);

    /// @brief Check if this entity has a component.
    /// @tparam ComponentType Component type for which to check existence.
    /// @return bool A boolean indicating component existence.
    template <typename ComponentType>
    [[nodiscard]] bool has_component() const;

    /// @brief Get a component stored in this entity, will return this first component that matches the specified type.
    /// @tparam ComponentType Component type to retrieve.
    /// @return A ComponentRef, or an empty ref if no component was found.
    template <typename ComponentType>
    [[nodiscard]] ComponentRef get_component() const;

    /// @brief Get all components associated with this entity.
    /// @return A vector of ComponentRefs associated with this entity.
    [[nodiscard]] std::vector<ComponentRef> const& get_components() const { return m_components; }

private:
    /// @brief Find a unique name for this entity in the parent entity.
    /// @param parent Parent node to use for unique name search, may be nullptr.
    /// @param name Candidate name to make unique.
    /// @return A guaranteed unique name in the given parent node.
    [[nodiscard]] static std::string get_unique_name_in_parent(Entity const* parent, std::string const& name);

private:
    std::string                 m_name;
    Entity*                     m_parent;
    std::vector<Ref>            m_children;
    Transform                   m_transform;
    std::vector<ComponentRef>   m_components;
};

#pragma region implementation

template <typename EntityType, typename... Args>
Entity::Ref Entity::create(Args&&... args)
{
    static_assert(std::is_base_of_v<Entity, EntityType> && "EntityType should derive from Entity!");
    return Ref(new EntityType(std::forward<Args>(args)...));
}

template <typename ComponentType, typename... Args>
void Entity::add_component(Args&&... args)
{
    static_assert(std::is_base_of_v<Component, ComponentType> && "ComponentType should derive from Component");
    ComponentRef component(new ComponentType(std::forward<Args>(args)...));
    component->set_entity(this);
    m_components.emplace_back(component);
}

template <typename ComponentType>
bool Entity::has_component() const
{
    for (auto const& component : m_components)
    {
        if (component != nullptr && typeid(*component) == typeid(ComponentType))
        {
            return true;
        }
    }

    return false;
}

template <typename ComponentType>
Entity::ComponentRef Entity::get_component() const
{
    for (auto const& component : m_components)
    {
        if (component != nullptr && typeid(*component) == typeid(ComponentType))
        {
            return component;
        }
    }

    return {};
}

#pragma endregion

#endif //BONSAI_RENDERER_ENTITY_HPP