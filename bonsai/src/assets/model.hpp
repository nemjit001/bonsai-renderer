#pragma once
#ifndef BONSAI_RENDERER_MODEL_HPP
#define BONSAI_RENDERER_MODEL_HPP

#include <string>
#include <vector>
#include "mesh.hpp"

/// @brief 3D model asset, contains mesh and material data used for rendering.
class Model
{
public:
    /// @brief Load a model asset from disk.
    /// @param path Path to the model asset.
    /// @return A new model asset.
    static Model from_file(std::string const& path);

    Model() = default;
    explicit Model(std::vector<Mesh> const& meshes)
        : m_meshes(meshes) {}
    ~Model() = default;

    Model(Model const&) = default;
    Model& operator=(Model const&) = default;

    std::vector<Mesh> const& meshes() const { return m_meshes; }

private:
    std::vector<Mesh> m_meshes{};
};

#endif //BONSAI_RENDERER_MODEL_HPP