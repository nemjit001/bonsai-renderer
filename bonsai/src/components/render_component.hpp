#pragma once
#ifndef BONSAI_RENDERER_RENDER_COMPONENT_HPP
#define BONSAI_RENDERER_RENDER_COMPONENT_HPP

#include "world/entity.hpp"
#include "assets/asset_cache.hpp"
#include "assets/model.hpp"

/// @brief The RenderComponent can be attached to entities to give them a model to render.
class RenderComponent : public Component
{
public:
    explicit RenderComponent(AssetHandle<Model> model);

    void set_model(AssetHandle<Model> model) { m_model = model; }
    [[nodiscard]] AssetHandle<Model> get_model() const { return m_model; }

private:
    AssetHandle<Model> m_model;
};

#endif //BONSAI_RENDERER_RENDER_COMPONENT_HPP