#include <gtest/gtest.h>
#include <assets/model.hpp>

TEST(assets, load_model)
{
    Model const model = Model::from_file("assets/CornellBox.obj");
    EXPECT_FALSE(model.meshes().empty());
}

TEST(assets, load_bad_model)
{
    Model const model = Model::from_file("assets/DOESNOTEXIST.obj");
    EXPECT_TRUE(model.meshes().empty());
}
