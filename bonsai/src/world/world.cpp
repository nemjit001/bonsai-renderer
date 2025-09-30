#include "world.hpp"

void World::update(double delta)
{
    m_root->update_tree(delta);
}
