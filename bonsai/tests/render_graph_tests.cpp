#include <gtest/gtest.h>
#include <render_graph/render_graph.hpp>

TEST(render_graph, single_pass)
{
    RenderGraph rg;

    RGResourceHandle buffer_resource = 0;
    RenderPass(&rg, "test pass")
        .write(buffer_resource)
        .commands([](RenderPassResources const&){});

    EXPECT_TRUE(rg.build());
}

TEST(render_graph, linear_dependencies)
{
    RenderGraph rg;

    RGResourceHandle buffer_resource = 0;
    RenderPass(&rg, "pass 1")
        .write(buffer_resource)
        .commands([](RenderPassResources const&){});

    RenderPass(&rg, "pass 2")
        .read(buffer_resource)
        .commands([](RenderPassResources const&){});

    EXPECT_TRUE(rg.build());
}

TEST(render_graph, shared_dependencies)
{
    RenderGraph rg;

    RGResourceHandle buffer_resource_a = 0;
    RGResourceHandle buffer_resource_b = 0;
    RGResourceHandle buffer_resource_c = 0;
    RenderPass(&rg, "pass 1")
        .write(buffer_resource_a)
        .commands([](RenderPassResources const&){});

    RenderPass(&rg, "pass 2")
        .write(buffer_resource_b)
        .commands([](RenderPassResources const&){});

    RenderPass(&rg, "pass 3")
        .read(buffer_resource_a)
        .read(buffer_resource_b)
        .write(buffer_resource_c)
        .commands([](RenderPassResources const&){});

    EXPECT_TRUE(rg.build());
}

TEST(render_graph, dependency_cycle)
{
    RenderGraph rg;

    RGResourceHandle buffer_resource_a = 0;
    RGResourceHandle buffer_resource_b = 0;

    RenderPass pass1(&rg, "pass 1");
    pass1.write(buffer_resource_a)
        .commands([](RenderPassResources const&){});

    RenderPass pass2(&rg, "pass 2");
    pass2.read(buffer_resource_a)
        .write(buffer_resource_b)
        .commands([](RenderPassResources const&){});

    pass1.read(buffer_resource_b);

    EXPECT_FALSE(rg.build());
}
