#include <gtest/gtest.h>
#include <world/world.hpp>

TEST(world, spawn_entity)
{
    World world{};
    world.get_root()->add_child(Entity::create<Entity>("child"));
    EXPECT_TRUE(world.get_root()->has_child("child"));
}

TEST(world, spawn_multiple_entities)
{
    World world{};
    Entity::Ref const root = world.get_root();
    root->add_child(Entity::create<Entity>("child1"));
    root->add_child(Entity::create<Entity>("child2"));
    EXPECT_TRUE(world.get_root()->has_child("child1"));
    EXPECT_TRUE(world.get_root()->has_child("child2"));
}

TEST(world, remove_entity)
{
    World world{};
    Entity::Ref const root = world.get_root();
    root->add_child(Entity::create<Entity>("child1"));
    root->add_child(Entity::create<Entity>("child2"));
    root->add_child(Entity::create<Entity>("child3"));
    root->remove_child("child2");

    EXPECT_TRUE(world.get_root()->has_child("child1"));
    EXPECT_TRUE(world.get_root()->has_child("child3"));
    EXPECT_FALSE(world.get_root()->has_child("child2"));
}

TEST(world, correctly_handle_name_collision)
{
    World world{};
    world.get_root()->add_child(Entity::create<Entity>("child"));
    world.get_root()->add_child(Entity::create<Entity>("child"));

    EXPECT_TRUE(world.get_root()->has_child("child"));
    EXPECT_TRUE(world.get_root()->has_child("child1"));
}

TEST(world, rename_node)
{
    World world{};
    world.get_root()->add_child(Entity::create<Entity>("child"));
    world.get_root()->add_child(Entity::create<Entity>("node"));
    world.get_root()->get_child("node")->set_name("child");

    EXPECT_TRUE(world.get_root()->has_child("child"));
    EXPECT_TRUE(world.get_root()->has_child("child1"));
}

TEST(world, move_entity)
{
    World world{};
    Entity::Ref const root = world.get_root();
    Entity::Ref const child1 = Entity::create<Entity>("child1");
    Entity::Ref const child2 = Entity::create<Entity>("child2");
    Entity::Ref const child3 = Entity::create<Entity>("child3");

    child1->add_child(child3);
    EXPECT_TRUE(child1->has_child("child3"));

    root->add_child(child1);
    root->add_child(child2);
    EXPECT_TRUE(world.get_root()->has_child("child1"));
    EXPECT_TRUE(world.get_root()->has_child("child2"));

    child2->add_child(child3);
    EXPECT_FALSE(child1->has_child("child3"));
    EXPECT_TRUE(child2->has_child("child3"));
}

TEST(world, iter_children_mutably)
{
    World world{};
    Entity::Ref const root = world.get_root();
    root->add_child(Entity::create<Entity>("child1"));
    root->add_child(Entity::create<Entity>("child2"));
    root->add_child(Entity::create<Entity>("child3"));
    EXPECT_EQ(root->get_children().size(), 3);

    for (auto& child : root->get_children())
    {
        child->set_name("node");
    }
    EXPECT_TRUE(root->has_child("node"));
    EXPECT_TRUE(root->has_child("node1"));
    EXPECT_TRUE(root->has_child("node2"));
}

TEST(world, add_component)
{
    World world{};
    world.get_root()->add_component<Component>();
    EXPECT_TRUE(world.get_root()->has_component<Component>());
    EXPECT_NE(world.get_root()->get_component<Component>(), nullptr);
}

TEST(world, add_custom_component)
{
    class CustomComponent : public Component
    {
        //
    };

    World world{};
    world.get_root()->add_component<CustomComponent>();
    EXPECT_TRUE(world.get_root()->has_component<CustomComponent>());
    EXPECT_NE(world.get_root()->get_component<CustomComponent>(), nullptr);
}

TEST(world, remove_component)
{
    World world{};
    world.get_root()->add_component<Component>();
    world.get_root()->add_component<Component>();

    world.get_root()->remove_component_by_index(0);
    EXPECT_TRUE(world.get_root()->has_component<Component>());
    EXPECT_NE(world.get_root()->get_component<Component>(), nullptr);

    world.get_root()->remove_component_by_index(0);
    EXPECT_FALSE(world.get_root()->has_component<Component>());
    EXPECT_EQ(world.get_root()->get_component<Component>(), nullptr);
}

TEST(world, single_timestep_modify_entity)
{
    class RemoverComponent : public Component
    {
    public:
        void update([[maybe_unused]] double delta) override
        {
            parent_entity().remove_child("child");
        }
    };

    World world{};
    world.get_root()->add_child(Entity::create<Entity>("child"));
    world.get_root()->add_component<RemoverComponent>();
    EXPECT_TRUE(world.get_root()->has_child("child"));

    world.update(0.0);
    EXPECT_FALSE(world.get_root()->has_child("child"));
}
