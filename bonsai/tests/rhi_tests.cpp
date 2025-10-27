#include <gtest/gtest.h>
#include <platform/platform.hpp>
#include <rhi/rhi.hpp>

class rhi_tests : public testing::Test
{
public:
    rhi_tests()
    {
        // Create rhi instance
        surface = platform.create_surface("rhi tests", 1280, 720, {});
        rhi_instance = rhi::create_instance();

        // Create render device
        RenderDeviceDesc render_device_desc{};
        render_device_desc.compatible_surface = surface;
        render_device = rhi_instance->create_render_device(render_device_desc);
    }

protected:
    Platform            platform;
    Surface*            surface;
    RHIInstanceHandle   rhi_instance;
    RenderDeviceHandle  render_device;
};

TEST_F(rhi_tests, create_render_device)
{
    EXPECT_NE(render_device, nullptr);
    EXPECT_EQ(render_device->is_headless(), false);
}

TEST_F(rhi_tests, create_buffer_resource)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 128;
    buffer_desc.usage = BufferUsageVertexBuffer | BufferUsageTransferDst;

    BufferHandle buffer = render_device->create_buffer(buffer_desc);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->size(), buffer_desc.size);
}

TEST_F(rhi_tests, create_texture_resource)
{
    TextureDesc texture_desc{};
    texture_desc.type = TextureType::Type2D;
    texture_desc.format = Format::RGBA32_FLOAT;
    texture_desc.width  = 128;
    texture_desc.height = 128;
    texture_desc.depth_or_layers = 1;
    texture_desc.mip_levels = 1;
    texture_desc.sample_count = 1;
    texture_desc.tiling = TextureTiling::Optimal;
    texture_desc.usage = TextureUsageSampled | TextureUsageTransferDst;

    TextureHandle texture = render_device->create_texture(texture_desc);
    EXPECT_NE(texture, nullptr);
}

TEST_F(rhi_tests, create_swapchain)
{
    SwapChainDesc swap_chain_desc{};
    swap_chain_desc.surface = surface;
    swap_chain_desc.image_count = 3;
    swap_chain_desc.format = Format::RGBA8_UNORM_SRGB;
    swap_chain_desc.width = 1280;
    swap_chain_desc.height = 720;
    swap_chain_desc.usage = TextureUsageColorAttachment;
    swap_chain_desc.present_mode = SwapPresentMode::FiFo;

    EXPECT_NE(render_device->create_swap_chain(swap_chain_desc), nullptr);
}
