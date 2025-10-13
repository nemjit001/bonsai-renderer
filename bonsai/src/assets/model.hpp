#pragma once
#ifndef BONSAI_RENDERER_MODEL_HPP
#define BONSAI_RENDERER_MODEL_HPP

#include <filesystem>
#include <vector>
#include "asset_cache.hpp"
#include "material.hpp"
#include "mesh.hpp"

/// @brief Model mesh segment, contains a mesh with an associated material.
struct ModelMesh
{
    Mesh        mesh;
    Material    material;
};

/// @brief 3D model asset, contains mesh and material data used for rendering.
class Model : public Asset
{
public:
    Model() = default;
    explicit Model(std::vector<ModelMesh> const& meshes)
        : m_meshes(meshes) {}
    ~Model() = default;

    Model(Model const&) = default;
    Model& operator=(Model const&) = default;

    std::vector<ModelMesh> const& meshes() const { return m_meshes; }

private:
    std::vector<ModelMesh> m_meshes{};
};

#endif //BONSAI_RENDERER_MODEL_HPP