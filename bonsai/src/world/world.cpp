#include "world.hpp"

Entity::Entity(std::string const& name)
    :
    m_name(name)
{
    //
}

void Entity::add_child(Entity const& entity)
{
    m_children.push_back(std::make_shared<Entity>(entity));
}
