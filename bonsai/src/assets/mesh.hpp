#pragma once
#ifndef BONSAI_RENDERER_MESH_HPP
#define BONSAI_RENDERER_MESH_HPP

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

/// @brief Default vertex layout for static meshes.
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 tex_coords;
};

/// @brief 3D mesh asset, contains geometry data for rendering.
class Mesh
{
public:
    Mesh() = default;
    Mesh(std::vector<Vertex> const& vertices, std::vector<uint32_t> const& indices)
        : m_vertices(vertices), m_indices(indices) {}
    ~Mesh() = default;

    Mesh(Mesh const&) = default;
    Mesh& operator=(Mesh const&) = default;

    std::vector<Vertex> const&      vertices() const    { return m_vertices; }
    std::vector<uint32_t> const&    indices() const     { return m_indices; }

private:
    std::vector<Vertex>     m_vertices;
    std::vector<uint32_t>   m_indices;
};

#endif //BONSAI_RENDERER_MESH_HPP