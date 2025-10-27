#include "render_graph.hpp"

#include "platform/assert.hpp"
#include "platform/logger.hpp"

RGResourceHandle RenderGraph::create_buffer(BufferDesc const& desc)
{
    ResourceMetaData resource{};
    resource.type = RGResourceType::Buffer;
    resource.version = 0;
    resource.config.buffer_desc = desc;

    uint32_t const id = static_cast<uint32_t>(m_graph_resources.size());
    m_graph_resources.push_back(resource);
    return id;
}

RGResourceHandle RenderGraph::create_texture(TextureDesc const& desc)
{
    ResourceMetaData resource{};
    resource.type = RGResourceType::Texture;
    resource.version = 0;
    resource.config.texture_desc = desc;

    uint32_t const id = static_cast<uint32_t>(m_graph_resources.size());
    m_graph_resources.push_back(resource);
    return id;
}

RGResourceHandle RenderGraph::import_buffer(BufferHandle buffer)
{
    ResourceMetaData resource{};
    resource.type = RGResourceType::ImportedBuffer;
    resource.version = 0;
    resource.handle.buffer_handle = std::move(buffer);

    uint32_t const id = static_cast<uint32_t>(m_graph_resources.size());
    m_graph_resources.push_back(resource);
    return id;
}

RGResourceHandle RenderGraph::import_texture(TextureHandle texture)
{
    ResourceMetaData resource{};
    resource.type = RGResourceType::ImportedTexture;
    resource.version = 0;
    resource.handle.texture_handle = std::move(texture);

    uint32_t const id = static_cast<uint32_t>(m_graph_resources.size());
    m_graph_resources.push_back(resource);
    return id;
}

RGBuildResult RenderGraph::build(RenderDeviceHandle& render_device)
{
    // Fill processing queue
    std::vector<RenderPassEntry> pass_queue;
    pass_queue.reserve(m_render_passes.size());
    for (auto const& [name, pass] : m_render_passes)
    {
        pass_queue.push_back(pass);
    }

    // Build out layered dependency graph using topological sort
    m_dependency_graph.clear();
    std::vector<RenderPassEntry> next_layer_queue;
    next_layer_queue.reserve(pass_queue.size());
    while (!pass_queue.empty())
    {
        next_layer_queue.clear();
        m_dependency_graph.emplace_back();
        auto& graph_layer = m_dependency_graph.back();
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
            return RGBuildResult::ErrorDependencyCycle; // No passes with no previous dependencies, indicates cycle in graph!
        }

        pass_queue = next_layer_queue;
    }

    // Allocate managed graph resources in single pass
    for (auto& resource : m_graph_resources)
    {
        if (resource.type == RGResourceType::Buffer && !resource.handle.buffer_handle)
        {
            resource.handle.buffer_handle = render_device->create_buffer(resource.config.buffer_desc);
            if (!resource.handle.buffer_handle)
            {
                return RGBuildResult::ErrorResourceAllocation;
            }
        }
        else if (resource.type == RGResourceType::Texture && !resource.handle.texture_handle)
        {
            resource.handle.texture_handle = render_device->create_texture(resource.config.texture_desc);
            if (!resource.handle.texture_handle)
            {
                return RGBuildResult::ErrorResourceAllocation;
            }
        }
    }

    return RGBuildResult::Success;
}

void RenderGraph::execute(ShaderDatabase const& shader_db, CommandBufferHandle& command_buffer) const
{
    for (auto const& layer : m_dependency_graph)
    {
        for (auto const& pass : layer)
        {
            // TODO(nemjit001): Transition pass resources here + set up RenderPassResources structure
            RenderPassResources pass_resources;
            if (pass.commands)
            {
                pass.commands(pass_resources, shader_db, command_buffer);
            }
        }
    }
}

void RenderGraph::clear()
{
    m_dependency_graph.clear();
    m_graph_resources.clear();
    m_dependency_graph.clear();
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

    if (resource >= m_graph_resources.size())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource read: resource {} does not exist", resource);
        return;
    }

    for (auto const& read_dependency : pass_iter->second.read_resources)
    {
        if (read_dependency.id == resource && read_dependency.usage == resource_usage)
        {
            BONSAI_LOG_WARNING(
                "Failed to add render pass resource read: resource usage {} does not match previous declared usage of {}",
                static_cast<uint32_t>(resource_usage),
                static_cast<uint32_t>(read_dependency.usage)
            );
            return;
        }
    }

    ResourceMetaData const& resource_data = m_graph_resources[resource];
    pass_iter->second.read_resources.push_back(VersionedResourceHandle{ resource, resource_data.version, resource_usage }); // TODO(nemjit001): Retrieve version from internal resource cache.
}

void RenderGraph::add_pass_resource_write(std::string const& name, RGResourceHandle const& resource, RGResourceUsage resource_usage)
{
    auto const pass_iter = m_render_passes.find(name);
    if (pass_iter == m_render_passes.end())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource write: pass {} does not exist", name);
        return;
    }

    if (resource >= m_graph_resources.size())
    {
        BONSAI_LOG_WARNING("Failed to add render pass resource write: resource {} does not exist", resource);
        return;
    }

    add_pass_resource_read(name, resource, resource_usage);
    ResourceMetaData& resource_data = m_graph_resources[resource];
    resource_data.version++;

    pass_iter->second.write_resources.push_back(VersionedResourceHandle{ resource, resource_data.version, resource_usage }); // TODO(nemjit001): Retrieve version from internal resource cache.
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

