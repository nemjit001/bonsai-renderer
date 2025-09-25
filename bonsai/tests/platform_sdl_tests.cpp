#include <gtest/gtest.h>
#include <platform/platform.hpp>

TEST(platform_sdl, create_default_surface)
{
    Platform platform{};
    Surface* surface = platform.create_surface("Test Surface", 512, 512, SurfaceConfig{});
    EXPECT_NE(surface, nullptr);

    platform.destroy_surface(surface);
}

TEST(platform_sdl, create_multiple_surfaces)
{
    Platform platform{};
    Surface* surface1 = platform.create_surface("Test Surface 1", 512, 512, SurfaceConfig{});
    Surface* surface2 = platform.create_surface("Test Surface 2", 512, 512, SurfaceConfig{});
    EXPECT_NE(surface1, nullptr);
    EXPECT_NE(surface2, nullptr);

    platform.destroy_surface(surface1);
    platform.destroy_surface(surface2);
}
