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

void Entity::set_name(std::string const& name)
{
    m_name = get_unique_name_in_parent(m_parent, name);
}

void Entity::add_child(Ref entity)
{
    // Remove from previous parent if it exists
    if (entity->m_parent != nullptr)
    {
        entity->m_parent->remove_child(entity->get_name());
    }

    // Update node name to be unique within entity parent/child relationship
    std::string const name = entity->get_unique_name_in_parent(this, entity->get_name());
    entity->set_name(name);

    // Set correct parent/child relationship
    entity->m_parent = this;
    m_children.emplace_back(std::move(entity));
}

void Entity::remove_child(std::string const& name)
{
    auto const it = std::find_if(std::begin(m_children), std::end(m_children), [&](Ref const& child)
    {
       return child->get_name() == name;
    });

    if (it != m_children.end())
    {
        m_children.erase(it);
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

Entity::Ref Entity::get_child(std::string const& name)
{
    for (auto const& child : m_children)
    {
        if (child->get_name() == name)
        {
            return child;
        }
    }

    return {};
}

glm::mat4 Entity::get_world_space_transform() const
{
    if (m_parent == nullptr)
    {
        return m_transform.matrix();
    }

    glm::mat4 const parent_transform = m_parent->get_world_space_transform();
    return m_transform.matrix() * parent_transform;
}

void Entity::update()
{
    for (auto const& child : m_children)
    {
        child->update();
    }
}

std::string Entity::get_unique_name_in_parent(Entity const* parent, std::string const& name)
{
    if (parent == nullptr)
    {
        return name;
    }

    int name_id = 0;
    std::string current_name = name;
    while (parent->has_child(current_name))
    {
        name_id += 1;
        current_name = name + std::to_string(name_id);
    }

    return current_name;
}
