#include <gtest/gtest.h>
#include <rhi/rhi.hpp>
#include <render_graph/render_graph.hpp>

class render_graph : public ::testing::Test
{
public:
    render_graph()
    {
        rhi_instance = create_rhi_instance();
        render_device = rhi_instance->create_render_device({});
        rg = RenderGraph();
    }

protected:
    RHIInstanceHandle   rhi_instance;
    RenderDeviceHandle  render_device;
    RenderGraph         rg;
};

TEST_F(render_graph, single_pass)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 1024;
    buffer_desc.usage = BufferUsageStorageBuffer;

    RGResourceHandle buffer_resource = rg.create_buffer(buffer_desc);
    RenderPass(&rg, "test pass")
        .write(buffer_resource);

    EXPECT_EQ(rg.build(render_device), RGBuildResult::Success);
}

TEST_F(render_graph, linear_dependencies)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 1024;
    buffer_desc.usage = BufferUsageStorageBuffer;

    RGResourceHandle buffer_resource = rg.create_buffer(buffer_desc);
    RenderPass(&rg, "pass 1")
        .write(buffer_resource);

    RenderPass(&rg, "pass 2")
        .read(buffer_resource);

    EXPECT_EQ(rg.build(render_device), RGBuildResult::Success);
}

TEST_F(render_graph, shared_dependencies)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 1024;
    buffer_desc.usage = BufferUsageStorageBuffer;

    RGResourceHandle buffer_resource_a = rg.create_buffer(buffer_desc);
    RGResourceHandle buffer_resource_b = rg.create_buffer(buffer_desc);
    RGResourceHandle buffer_resource_c = rg.create_buffer(buffer_desc);
    RenderPass(&rg, "pass 1")
        .write(buffer_resource_a);

    RenderPass(&rg, "pass 2")
        .write(buffer_resource_b);

    RenderPass(&rg, "pass 3")
        .read(buffer_resource_a)
        .read(buffer_resource_b)
        .write(buffer_resource_c);

    EXPECT_EQ(rg.build(render_device), RGBuildResult::Success);
}

TEST_F(render_graph, dependency_cycle)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 1024;
    buffer_desc.usage = BufferUsageStorageBuffer;

    RGResourceHandle buffer_resource_a = rg.create_buffer(buffer_desc);
    RGResourceHandle buffer_resource_b = rg.create_buffer(buffer_desc);
    RGResourceHandle buffer_resource_c = rg.create_buffer(buffer_desc);

    RenderPass pass1(&rg, "pass 1");
    pass1.write(buffer_resource_a);

    RenderPass pass2(&rg, "pass 2");
    pass2.read(buffer_resource_a)
        .write(buffer_resource_b);

    RenderPass pass3(&rg, "pass 3");
    pass3.read(buffer_resource_b)
        .write(buffer_resource_c);

    pass1.read(buffer_resource_c);
    EXPECT_EQ(rg.build(render_device), RGBuildResult::ErrorDependencyCycle);
}

TEST_F(render_graph, import_resources)
{
    BufferDesc buffer_desc{};
    buffer_desc.size = 1024;
    buffer_desc.usage = BufferUsageStorageBuffer;

    BufferHandle imported_buffer = render_device->create_buffer(buffer_desc);
    RGResourceHandle buffer_resource = rg.import_buffer(imported_buffer);

    RenderPass(&rg, "test pass")
        .write(buffer_resource);

    EXPECT_EQ(rg.build(render_device), RGBuildResult::Success);
}
