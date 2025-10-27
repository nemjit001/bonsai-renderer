#include <gtest/gtest.h>
#include <assets/asset_cache.hpp>
#include <assets/model.hpp>

TEST(assets_tests, create_model)
{
    AssetHandle<Model> const test_model = AssetCache::create<Model>("test_model");
    AssetHandle<Model> const cached_model = AssetCache::load<Model>("test_model");
    EXPECT_EQ(test_model, cached_model);
}

TEST(assets_tests, load_bad_model)
{
    AssetHandle<Model> const model = AssetCache::load<Model>("DOESNOTEXIST.obj");
    EXPECT_TRUE(model->meshes().empty());
}
