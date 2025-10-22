#pragma once
#ifndef BONSAI_RENDERER_MATERIAL_HPP
#define BONSAI_RENDERER_MATERIAL_HPP

#include <glm/glm.hpp>

/// @brief PBR material specification for model files.
class Material
{
public:
    glm::vec3 diffuse = glm::vec3(0.5, 0.5, 0.5);
};

#endif //BONSAI_RENDERER_MATERIAL_HPP