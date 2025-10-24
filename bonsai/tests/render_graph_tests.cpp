#include <gtest/gtest.h>
#include <render_graph/render_graph.hpp>

TEST(render_graph, single_pass)
{
    RenderGraph rg;
    RenderPass(&rg, "test pass")
        .commands([](RenderPassResources const&){});
    EXPECT_TRUE(rg.build());
}

TEST(render_graph, linear_dependencies)
{
    RenderGraph rg;
    RenderPass(&rg, "pass 1")
        .commands([](RenderPassResources const&){});
    RenderPass(&rg, "pass 2")
        .commands([](RenderPassResources const&){});
    EXPECT_TRUE(rg.build());
}
