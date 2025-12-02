#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_HPP

#include <cstdint>
#include <cstddef>
#include "bonsai/core/platform.hpp"

/// @brief Render backend new frame or present result indicating frame state.
enum class RenderBackendFrameResult
{
    Ok = 0,
    SwapOutOfDate,
    FatalError,
};

/// @brief Render buffer usage flag bits.
enum RenderBufferUsage
{
    BufferUsageTransferSrc      = 0x01,
    BufferUsageTransferDst      = 0x02,
    BufferUsageUniformBuffer    = 0x04,
    BufferUsageStorageBuffer    = 0x08,
    BufferUsageIndexBuffer      = 0x10,
    BufferUsageVertexBuffer     = 0x20,
    BufferUsageIndirectBuffer   = 0x40,
};
typedef uint32_t RenderBufferUsageFlags;

/// @brief The RenderBuffer represents a backend buffer type that can store data.
class RenderBuffer
{
public:
    virtual ~RenderBuffer() = default;

    /// @brief Map this buffer to host visible memory.
    /// @param data Data pointer to use for mapped region.
    /// @param size Size of buffer to map.
    /// @param offset Offset into buffer to start mapped region at.
    [[nodiscard]]
    virtual bool map(void** data, size_t size, size_t offset) = 0;

    /// @brief Unmap this buffer from host visible memory.
    virtual void unmap() = 0;
};

/// @brief The RenderTexture represents a backend texture type.
class RenderTexture
{
public:
    virtual ~RenderTexture() = default;
};

/// @brief The ShaderPipeline represents a backend shader pipeline that can be used for rendering.
class ShaderPipeline
{
public:
    /// @brief Shader pipeline types.
    enum PipelineType
    {
        None        = 0,
        Graphics    = 1,
        Compute     = 2,
    };

    /// @brief Compute shader workgroup size for the shader pipeline.
    struct WorkgroupSize
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

public:
    virtual ~ShaderPipeline() = default;

    /// @brief Get the shader pipeline type.
    /// @return The type of shader pipeline.
    [[nodiscard]]
    PipelineType get_type() const { return m_type; }

    /// @brief Get the shader pipeline workgroup size, if it was specified in the shader.
    /// Only compute shaders specify a workgroup size.
    /// @return The workgroup size for the shader.
    [[nodiscard]]
    WorkgroupSize get_workgroup_size() const { return m_workgroup_size; }

private:
    PipelineType m_type = PipelineType::None;
    WorkgroupSize m_workgroup_size = { 0, 0, 0 };
};

/// @brief The RenderCommands class is used for recording render backend commands.
class RenderCommands
{
public:
    virtual ~RenderCommands() = default;

    /// @brief Begin command recording.
    /// @return A boolean indicating successful command recording start.
    [[nodiscard]]
    virtual bool begin() = 0;

    /// @brief End the command recording.
    /// @return A boolean indicating successful command recording end.
    [[nodiscard]]
    virtual bool end() = 0;

    /// @brief Set the currently active shader pipeline.
    /// @param pipeline Pipeline to activate.
    virtual void set_pipeline(ShaderPipeline* pipeline) = 0;

    /// @brief Bind a uniform buffer to a named shader location.
    /// @param name Binding location name.
    /// @param buffer Uniform buffer to bind.
    /// @param size Buffer size to bind.
    /// @param offset Buffer offset to bind.
    virtual void bind_uniform(char const* name, RenderBuffer* buffer, size_t size, size_t offset) = 0;

    /// @brief Bind a storage buffer to a named shader location.
    /// @param name Binding location name.
    /// @param buffer Buffer to bind.
    /// @param size Buffer size to bind.
    /// @param offset Buffer offset to bind.
    virtual void bind_buffer(char const* name, RenderBuffer* buffer, size_t size, size_t offset) = 0;

    /// @brief Bind a texture to a named shader location.
    /// @param name Binding location name.
    /// @param texture Texture to bind.
    virtual void bind_texture(char const* name, RenderTexture* texture) = 0;

    /// @brief Dispatch compute workgroups using the active compute pipeline.
    /// @param x Dispatch dimension x.
    /// @param y Dispatch dimension y.
    /// @param z Dispatch dimension z.
    virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;
};

/// @brief The RenderBackend wraps a backend graphics API, providing a common interface for the engine to use.
class RenderBackend
{
protected:
    RenderBackend() = default;
public:
    virtual ~RenderBackend() = default;

    /// @brief Create a render backend.
    /// @param platform_surface Main surface to use for rendering, will be used to initialize the render backend.
    /// @return A new render backend, or nullptr if no backend is active.
    static RenderBackend* create(PlatformSurface* platform_surface);

    /// @brief Wait for the backend render device to be idle.
    virtual void wait_idle() const = 0;

    /// @brief Start a new render backend frame.
    /// @return A render backend frame result.
    [[nodiscard]]
    virtual RenderBackendFrameResult new_frame() = 0;

    /// @brief End the currently active render backend frame.
    /// @return A render backend frame result.
    [[nodiscard]]
    virtual RenderBackendFrameResult end_frame() = 0;

    /// @brief Get the currently active frame render commands.
    /// @return A RenderCommands structure for recording frame commands.
    [[nodiscard]]
    virtual RenderCommands* get_frame_commands() = 0;

    /// @brief Create a render buffer.
    /// @param buffer_usage Buffer usage flags.
    /// @param size Buffer size in bytes.
    /// @param can_map Indicates if this buffer can be mapped to host memory.
    [[nodiscard]]
    virtual RenderBuffer* create_buffer(RenderBufferUsageFlags buffer_usage, size_t size, bool can_map) = 0;

    [[nodiscard]]
    virtual RenderTexture* create_texture() = 0;

    /// @brief Get the current frame index.
    /// @return The currently active frame index.
    [[nodiscard]]
    virtual uint64_t get_current_frame_index() const = 0;
};

#endif //BONSAI_RENDERER_RENDER_BACKEND_HPP