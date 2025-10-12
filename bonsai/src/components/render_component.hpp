#pragma once
#ifndef BONSAI_RENDERER_RENDER_COMPONENT_HPP
#define BONSAI_RENDERER_RENDER_COMPONENT_HPP

#include "world/entity.hpp"
#include "assets/model.hpp"

/// @brief The RenderComponent can be attached to entities to give them a model to render.
class RenderComponent : public Component
{
public:
    explicit RenderComponent(Model const& model);

    void set_model(Model const& model) { m_model = model; }
    [[nodiscard]] Model const& get_model() const { return m_model; }

private:
    Model m_model;
};

#endif //BONSAI_RENDERER_RENDER_COMPONENT_HPP