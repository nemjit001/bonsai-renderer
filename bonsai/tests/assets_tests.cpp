#include <gtest/gtest.h>
#include <assets/asset_cache.hpp>
#include <assets/model.hpp>

TEST(assets, load_model)
{
    AssetHandle<Model> const model = AssetCache::load<Model>("assets/CornellBox.obj");
    EXPECT_FALSE(model->meshes().empty());
}

TEST(assets, unload_model)
{
    AssetHandle<Model> const model = AssetCache::load<Model>("assets/CornellBox.obj");
    EXPECT_FALSE(model->meshes().empty());
    AssetCache::unload(model);
}

TEST(assets, load_bad_model)
{
    AssetHandle<Model> const model = AssetCache::load<Model>("assets/DOESNOTEXIST.obj");
    EXPECT_TRUE(model->meshes().empty());
}
