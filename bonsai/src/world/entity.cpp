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
    m_parent(nullptr),
    m_transform(transform)
{
    //
}

void Entity::add_child(Ref entity)
{
    // Remove from previous parent if it exists
    if (entity->m_parent != nullptr)
    {
        entity->m_parent->remove_child(entity->get_name());
    }

    // Set correct parent/child relationship
    entity->m_parent = this;
    m_children.emplace_back(std::move(entity));
}

void Entity::remove_child(std::string const& name)
{
    auto it = m_children.begin();
    while (it != m_children.end())
    {
        Ref child = *it;
        if (child->get_name() == name)
        {
            child->m_parent = nullptr;
            it = m_children.erase(it);
        }
        else
        {
            it++;
        }
    }
}

bool Entity::has_child(std::string const& name) const
{
    for (auto const& child : m_children)
    {
        if (child->get_name() == name)
        {
            return true;
        }
    }

    return false;
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
