#include <gtest/gtest.h>
#include <world/world.hpp>

TEST(world, spawn_entity)
{
    World world{};
    world.get_root()->add_child(Entity::create<Entity>("child"));
    ASSERT_TRUE(world.get_root()->has_child("child"));
}

TEST(world, spawn_multiple_entities)
{
    World world{};
    Entity::Ref const root = world.get_root();
    root->add_child(Entity::create<Entity>("child1"));
    root->add_child(Entity::create<Entity>("child2"));
    ASSERT_TRUE(world.get_root()->has_child("child1"));
    ASSERT_TRUE(world.get_root()->has_child("child2"));
}

TEST(world, remove_entity)
{
    World world{};
    Entity::Ref const root = world.get_root();
    root->add_child(Entity::create<Entity>("child1"));
    root->add_child(Entity::create<Entity>("child2"));
    root->add_child(Entity::create<Entity>("child3"));
    root->remove_child("child2");

    ASSERT_TRUE(world.get_root()->has_child("child1"));
    ASSERT_TRUE(world.get_root()->has_child("child3"));
    ASSERT_FALSE(world.get_root()->has_child("child2"));
}

TEST(world, move_entity)
{
    World world{};
    Entity::Ref const root = world.get_root();
    Entity::Ref const child1 = Entity::create<Entity>("child1");
    Entity::Ref const child2 = Entity::create<Entity>("child2");
    Entity::Ref const child3 = Entity::create<Entity>("child3");

    child1->add_child(child3);
    ASSERT_TRUE(child1->has_child("child3"));

    root->add_child(child1);
    root->add_child(child2);
    ASSERT_TRUE(world.get_root()->has_child("child1"));
    ASSERT_TRUE(world.get_root()->has_child("child2"));

    child2->add_child(child3);
    ASSERT_FALSE(child1->has_child("child3"));
    ASSERT_TRUE(child2->has_child("child3"));
}
