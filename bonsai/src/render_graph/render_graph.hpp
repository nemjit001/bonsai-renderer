#pragma once
#ifndef BONSAI_RENDERER_RENDER_GRAPH_HPP
#define BONSAI_RENDERER_RENDER_GRAPH_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

class RenderPassResources;

/// @brief Thin resource handle to uniquely identify resources in the render graph.
using RGResourceHandle = uint32_t;

/// @brief Render pass command recording function.
using RenderPassCommands = std::function<void(RenderPassResources const&)>;

/// @brief Render graph resource types.
enum class RGResourceType : uint8_t
{
    Buffer,
    Texture,
    ImportedBuffer,
    ImportedTexture,
};

/// @brief Retained mode render graph, stores render pass and resource state for a frame.
class RenderGraph
{
public:
    friend class RenderPass;

    /// @brief Build the render graph.
    /// @return True on successful build, false otherwise.
    [[nodiscard]] bool build();

    /// @brief Execute the render graph. The graph needs to be built before execution.
    void execute();

private:
    /// @brief Insert a new named render pass.
    /// @param name Render pass name.
    void insert_render_pass(std::string const& name);

    /// @brief Add a resource read to a pass.
    /// @param name Pass name.
    /// @param resource
    void add_pass_resource_read(std::string const& name, RGResourceHandle const& resource);

    /// @brief Add a resource write to a pass.
    /// @param name Pass name.
    /// @param resource
    void add_pass_resource_write(std::string const& name, RGResourceHandle const& resource);

    /// @brief Set render pass commands.
    /// @param name Pass name.
    /// @param commands Pass command function.
    void set_pass_commands(std::string const& name, RenderPassCommands const& commands);

private:
    /// @brief Versioned resource handle to track read/write dependencies in render pass entries.
    struct VersionedResourceHandle
    {
        uint32_t id;
        uint32_t version;
    };

    /// @brief Internal render pass entry state.
    struct RenderPassEntry
    {
        std::vector<VersionedResourceHandle> read_resources;
        std::vector<VersionedResourceHandle> write_resources;
        RenderPassCommands commands;
    };

    std::unordered_map<std::string, RenderPassEntry> m_render_passes;
};

/// @brief Resources available to a single render pass, declared using the RenderPass read/write functionality.
class RenderPassResources
{
public:
    //
};

/// @brief A RenderPass describes a set of graphics commands that together build a render pass.
class RenderPass
{
public:
    /// @brief Create a new RenderPass.
    /// @param render_graph Render graph to create this pass in.
    /// @param name Render pass name to uniquely identify this pass within the graph.
    RenderPass(RenderGraph* render_graph, std::string const& name);

    /// @brief Add a resource read in this pass, will make resource available in RenderPassResources.
    /// @param resource Resource to read.
    /// @return
    RenderPass& read(RGResourceHandle const& resource);

    /// @brief Add a resource write in this pass, will make resource available in RenderPassResources.
    /// @param resource Resource to write, will add a read dependency too.
    /// @return
    RenderPass& write(RGResourceHandle const& resource);

    /// @brief Set the render commands for this pass.
    /// @param commands Render commands recorded into a command buffer.
    void commands(RenderPassCommands const& commands);

private:
    RenderGraph*    m_render_graph;
    std::string     m_name;
};

#endif //BONSAI_RENDERER_RENDER_GRAPH_HPP