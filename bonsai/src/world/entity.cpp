#include "entity.hpp"

Entity::Entity(std::string const& name)
    :
    m_name(name)
{
    //
}

void Entity::add_child(Ref entity)
{
    m_children.emplace_back(std::move(entity));
}
