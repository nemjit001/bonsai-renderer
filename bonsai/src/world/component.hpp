#pragma once
#ifndef BONSAI_RENDERER_COMPONENT_HPP
#define BONSAI_RENDERER_COMPONENT_HPP

/// @brief Component interface, can be added to an entity in the world to provide behaviour.
class Component
{
public:
    virtual ~Component() = default;

    /// @brief Update this component's state.
    /// @param delta Time delta between updates.
    virtual void update([[maybe_unused]] double delta) {};
};

#endif //BONSAI_RENDERER_COMPONENT_HPP