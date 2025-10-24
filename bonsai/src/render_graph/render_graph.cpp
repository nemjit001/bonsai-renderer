#include "render_graph.hpp"

#include "platform/assert.hpp"
#include "platform/logger.hpp"

bool RenderGraph::build()
{
    return true;
}

void RenderGraph::execute()
{
    //
}

void RenderGraph::insert_render_pass(std::string const& name)
{
    m_render_passes.insert({ name, RenderPassEntry{} });
}


void RenderGraph::add_pass_resource_read(std::string const& name, RGResourceHandle const& resource)
{
    auto const pass_iter = m_render_passes.find(name);
    if (pass_iter == m_render_passes.end())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource read: pass {} does not exist", name);
        return;
    }

    pass_iter->second.read_resources.push_back(VersionedResourceHandle{ resource, 0 }); // TODO(nemjit001): Retrieve version from internal resource cache.
}

void RenderGraph::add_pass_resource_write(std::string const& name, RGResourceHandle const& resource)
{
    auto const pass_iter = m_render_passes.find(name);
    if (pass_iter == m_render_passes.end())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource write: pass {} does not exist", name);
        return;
    }

    add_pass_resource_read(name, resource);
    pass_iter->second.write_resources.push_back(VersionedResourceHandle{ resource, 0 }); // TODO(nemjit001): Retrieve version from internal resource cache.
}

void RenderGraph::set_pass_commands(std::string const& name, RenderPassCommands const& commands)
{
    auto const pass_iter = m_render_passes.find(name);
    if (pass_iter == m_render_passes.end())
    {
        BONSAI_LOG_WARNING("Failed to set render pass commands: pass {} does not exist", name);
        return;
    }
    pass_iter->second.commands = commands;
}

RenderPass::RenderPass(RenderGraph* render_graph, std::string const& name)
    :
    m_render_graph(render_graph),
    m_name(name)
{
    BONSAI_ASSERT(m_render_graph != nullptr && "RenderGraph was nullptr!");
    m_render_graph->insert_render_pass(name);
}

RenderPass& RenderPass::read(RGResourceHandle const& resource)
{
    m_render_graph->add_pass_resource_read(m_name, resource);
    return *this;
}

RenderPass& RenderPass::write(RGResourceHandle const& resource)
{
    m_render_graph->add_pass_resource_write(m_name, resource);
    return *this;
}

void RenderPass::commands(RenderPassCommands const& commands)
{
    m_render_graph->set_pass_commands(m_name, commands);
}

