#include <gtest/gtest.h>
#include <platform/platform.hpp>

TEST(platform, create_default_surface)
{
    Platform platform{};
    Surface* surface = platform.create_surface("Test Surface", 512, 512, SurfaceConfig{});
    EXPECT_NE(surface, nullptr);

    platform.destroy_surface(surface);
}

TEST(platform, create_multiple_surfaces)
{
    Platform platform{};
    Surface* surface1 = platform.create_surface("Test Surface 1", 512, 512, SurfaceConfig{});
    Surface* surface2 = platform.create_surface("Test Surface 2", 512, 512, SurfaceConfig{});
    EXPECT_NE(surface1, nullptr);
    EXPECT_NE(surface2, nullptr);

    platform.destroy_surface(surface1);
    platform.destroy_surface(surface2);
}

TEST(platform, query_surface_size)
{
    Platform platform{};
    Surface* surface = platform.create_surface("Test Surface", 512, 256, SurfaceConfig{});
    EXPECT_NE(surface, nullptr);

    uint32_t width = 0, height = 0;
    surface->get_size(width, height);
    EXPECT_EQ(width, 512);
    EXPECT_EQ(height, 256);

    platform.destroy_surface(surface);
}
