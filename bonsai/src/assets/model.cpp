#include "model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include "core/assert.hpp"
#include "core/logger.hpp"

Model Model::from_file(std::string const& path)
{
    tinyobj::ObjReaderConfig reader_config{};
    reader_config.triangulate = true;
    reader_config.triangulation_method = "earcut";

    tinyobj::ObjReader reader{};
    if (!reader.ParseFromFile(path, reader_config))
    {
        std::string const error = reader.Error().empty() ? "Unknown Error" : reader.Error();
        BONSAI_LOG_ERROR("Failed to load OBJ file: {}", error);
        return {};
    }

    if (!reader.Warning().empty())
    {
        BONSAI_LOG_WARNING("{}", reader.Warning());
    }

    auto const& attrib = reader.GetAttrib();
    auto const& materials = reader.GetMaterials();
    auto const& shapes = reader.GetShapes();

    std::vector<Mesh> model_meshes{};
    model_meshes.reserve(shapes.size());
    for (auto const& shape : shapes)
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        for (auto const& face_vertices : shape.mesh.num_face_vertices)
        {
            BONSAI_ASSERT(face_vertices == 3 && "OBJ mesh data is not triangulated!");
            glm::vec3 face_positions[3]{};
            glm::vec3 face_normals[3]{};
            glm::vec2 face_texcoords[3]{};
            for (size_t f = 0; f < face_vertices; f++)
            {
                tinyobj::index_t const& index = shape.mesh.indices[f];
                BONSAI_ASSERT((index.normal_index == -1 || index.texcoord_index == -1) && "Bonsai mesh loading requires normals and texcoords to be available!");
                size_t const vertex_idx = index.vertex_index * 3;
                size_t const normal_idx = index.normal_index * 3;
                size_t const texcoord_idx = index.texcoord_index * 2;

                face_positions[f] = glm::vec3{ attrib.vertices[vertex_idx + 0], attrib.vertices[vertex_idx + 1], attrib.vertices[vertex_idx + 2] };
                face_normals[f] = glm::vec3{ attrib.normals[normal_idx + 0], attrib.normals[normal_idx + 1], attrib.normals[normal_idx + 2] };
                face_texcoords[f] = glm::vec2{ attrib.texcoords[texcoord_idx + 0], attrib.texcoords[texcoord_idx + 1] };
            }

            // Calculates face tangent from vertex positions & texcoords
            glm::vec3 const e1 = face_positions[1] - face_positions[0];
            glm::vec3 const e2 = face_positions[2] - face_positions[0];
            glm::vec2 const dUV1 = face_texcoords[1] - face_texcoords[0];
            glm::vec2 const dUV2 = face_texcoords[2] - face_texcoords[0];
            float const f = 1.0F / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
            glm::vec3 face_tangent = f * (dUV2.y * e1 - dUV1.y * e2);

            // Push indexed vertex data into mesh buffers
            for (size_t f = 0; f < face_vertices; f++)
            {
                vertices.emplace_back(Vertex{
                    face_positions[f],
                    face_normals[f],
                    face_tangent,
                    face_texcoords[f]
                });
                indices.push_back(static_cast<uint32_t>(indices.size()));
            }
        }

        model_meshes.emplace_back(vertices, indices);
    }

    return Model(model_meshes);
}
