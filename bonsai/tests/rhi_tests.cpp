#include <gtest/gtest.h>
#include <rhi/rhi.hpp>

class rhi : public ::testing::Test
{
public:
    rhi()
    {
        // Create rhi instance
        rhi_instance = create_rhi_instance();

        // Create headless render device
        RenderDeviceDesc render_device_desc{};
        render_device = rhi_instance->create_render_device(render_device_desc);
    }

protected:
    RHIInstanceHandle   rhi_instance;
    RenderDeviceHandle  render_device;
};

TEST_F(rhi, create_render_device)
{
    EXPECT_NE(render_device, nullptr);
    EXPECT_EQ(render_device->is_headless(), true);
}

TEST_F(rhi, create_buffer_resource)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 128;
    buffer_desc.usage = BufferUsageVertexBuffer | BufferUsageTransferDst;

    BufferHandle buffer = render_device->create_buffer(buffer_desc);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->size(), buffer_desc.size);
}

TEST_F(rhi, create_texture_resource)
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
