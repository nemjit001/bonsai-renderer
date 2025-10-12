#include "camera_component.hpp"

Camera::Camera(float fov, float z_near, float z_far)
    :
    fov(fov),
    z_near(z_near),
    z_far(z_far)
{
    //
}

CameraComponent::CameraComponent(Camera const& camera)
    :
    m_camera(camera)
{
    //
}
