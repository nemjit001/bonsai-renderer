#include <gtest/gtest.h>
#include <world/world.hpp>

TEST(world, spawn_entity)
{
    World world{};
    world.get_root()->add_child(Entity::create<Entity>("child"));
}
