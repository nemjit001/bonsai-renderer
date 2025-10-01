#pragma once
#ifndef BONSAI_RENDERER_COMPONENT_HPP
#define BONSAI_RENDERER_COMPONENT_HPP

class Entity;

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

#endif //BONSAI_RENDERER_COMPONENT_HPP