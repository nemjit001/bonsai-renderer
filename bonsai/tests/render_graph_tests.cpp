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
