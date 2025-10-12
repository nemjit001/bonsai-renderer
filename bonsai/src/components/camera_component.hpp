#pragma once
#ifndef BONSAI_RENDERER_CAMERA_COMPONENT_HPP
#define BONSAI_RENDERER_CAMERA_COMPONENT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "world/entity.hpp"

/// @brief Virtual camera type used for rendering.
class Camera
{
public:
    Camera() = default;
    Camera(float fov, float z_near, float z_far);

    /// @brief Get the camera projection matrix based on its parameters.
    [[nodiscard]] inline glm::mat4 projection_matrix() const;

public:
    float fov       = 60.0F;
    float z_near    = 0.001F;
    float z_far     = 100.0F;
};

/// @brief The CameraComponent can be attached to entities that should act as virtual views into the world.
class CameraComponent : public Component
{
public:
    explicit CameraComponent(Camera const& camera);

    void set_camera(Camera const& camera) { m_camera = camera; }
    [[nodiscard]] Camera const& get_camera() const { return m_camera; }

private:
    Camera m_camera;
};

#pragma region implementation

glm::mat4 Camera::projection_matrix() const
{
    return glm::perspective(fov, 1.0F, z_near, z_far); // FIXME(nemjit001): Set aspect ratio according to viewport size
}

#pragma endregion

#endif //BONSAI_RENDERER_CAMERA_COMPONENT_HPP