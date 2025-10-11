#pragma once
#ifndef BONSAI_RENDERER_MODEL_HPP
#define BONSAI_RENDERER_MODEL_HPP

#include <string>
#include <vector>
#include "material.hpp"
#include "mesh.hpp"

/// @brief Model mesh segment, contains a mesh with an associated material.
struct ModelMesh
{
    Mesh        mesh;
    Material    material;
};

/// @brief 3D model asset, contains mesh and material data used for rendering.
class Model
{
public:
    /// @brief Load a model asset from disk.
    /// @param path Path to the model asset.
    /// @return A new model asset.
    static Model from_file(std::string const& path);

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