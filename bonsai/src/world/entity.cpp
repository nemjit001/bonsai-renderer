#include "entity.hpp"

glm::mat4 Transform::matrix() const
{
    return glm::translate(glm::identity<glm::mat4>(), position)
        * glm::mat4_cast(rotation)
        * glm::scale(glm::identity<glm::mat4>(), scale);
}

Entity::Entity(std::string const& name)
    :
    m_name(name),
    m_parent(nullptr)
{
    //
}

Entity::Entity(std::string const& name, Transform const& transform)
    :
    m_name(name),
    m_transform(transform),
    m_parent(nullptr)
{
    //
}

void Entity::add_child(Ref entity)
{
    entity->m_parent = this;
    m_children.emplace_back(std::move(entity));
}

glm::mat4 Entity::get_worldspace_transform() const
{
    if (m_parent == nullptr)
    {
        return m_transform.matrix();
    }

    glm::mat4 const parent_transform = m_parent->get_worldspace_transform();
    return m_transform.matrix() * parent_transform;
}
