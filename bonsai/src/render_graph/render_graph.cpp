#include "render_graph.hpp"

#include "platform/assert.hpp"
#include "platform/logger.hpp"

bool RenderGraph::build()
{
    std::vector<RenderPassEntry> pass_queue;
    pass_queue.reserve(m_render_passes.size());
    for (auto const& [name, pass] : m_render_passes)
    {
        pass_queue.push_back(pass);
    }

    std::vector<std::vector<RenderPassEntry>> dependency_graph;
    std::vector<RenderPassEntry> next_layer_queue;
    next_layer_queue.reserve(pass_queue.size());
    while (!pass_queue.empty())
    {
        next_layer_queue.clear();
        dependency_graph.emplace_back();
        auto& graph_layer = dependency_graph.back();
        for (auto const& pass : pass_queue)
        {
            if (find_pass_dependency_count(pass, pass_queue) == 0)
            {
                graph_layer.push_back(pass);
            }
            else
            {
                next_layer_queue.push_back(pass);
            }
        }

        if (graph_layer.empty())
        {
            BONSAI_LOG_ERROR("Detected a cycle in the Render Graph!");
            return false; // No passes with parents available, indicates cycle in graph!
        }

        pass_queue = next_layer_queue;
    }

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

void RenderGraph::add_pass_resource_read(std::string const& name, RGResourceHandle const& resource, RGResourceUsage resource_usage)
{
    auto const pass_iter = m_render_passes.find(name);
    if (pass_iter == m_render_passes.end())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource read: pass {} does not exist", name);
        return;
    }

    pass_iter->second.read_resources.push_back(VersionedResourceHandle{ resource, 0, resource_usage }); // TODO(nemjit001): Retrieve version from internal resource cache.
}

void RenderGraph::add_pass_resource_write(std::string const& name, RGResourceHandle const& resource, RGResourceUsage resource_usage)
{
    auto const pass_iter = m_render_passes.find(name);
    if (pass_iter == m_render_passes.end())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource write: pass {} does not exist", name);
        return;
    }

    add_pass_resource_read(name, resource, resource_usage);
    pass_iter->second.write_resources.push_back(VersionedResourceHandle{ resource, 1, resource_usage }); // TODO(nemjit001): Retrieve version from internal resource cache.
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

int32_t RenderGraph::find_pass_dependency_count(RenderPassEntry const& entry, std::vector<RenderPassEntry> const& pass_queue)
{
    int32_t dependency_count = 0;
    for (auto const& read_dependency : entry.read_resources)
    {
        for (auto const& entry : pass_queue)
        {
            for (auto const& write_dependency : entry.write_resources)
            {
                if (read_dependency.id == write_dependency.id
                    && read_dependency.version == write_dependency.version)
                {
                    dependency_count++;
                }
            }
        }
    }

    return dependency_count;
}

RenderPass::RenderPass(RenderGraph* render_graph, std::string const& name)
    :
    m_render_graph(render_graph),
    m_name(name)
{
    BONSAI_ASSERT(m_render_graph != nullptr && "RenderGraph was nullptr!");
    m_render_graph->insert_render_pass(name);
}

RenderPass& RenderPass::read(RGResourceHandle const& resource, RGResourceUsage resource_usage)
{
    m_render_graph->add_pass_resource_read(m_name, resource, resource_usage);
    return *this;
}

RenderPass& RenderPass::write(RGResourceHandle const& resource, RGResourceUsage resource_usage)
{
    m_render_graph->add_pass_resource_write(m_name, resource, resource_usage);
    return *this;
}

void RenderPass::commands(RenderPassCommands const& commands)
{
    m_render_graph->set_pass_commands(m_name, commands);
}

