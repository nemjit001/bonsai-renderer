#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_HPP

#include <cstdint>
#include <cstddef>
#include <imgui.h>
#include "bonsai/core/platform.hpp"

/// @brief Render backend new frame or present result indicating frame state.
enum class RenderBackendFrameResult
{
    Ok = 0,
    SwapOutOfDate,
    FatalError,
};

/// @brief Render backend format types for binary data such as textures and buffers.
enum RenderFormat : uint32_t
{
    RenderFormatUndefined = 0,
    // TODO(nemjit001): Fill out common formats for backends
};

/// @brief Render buffer usage flag bits.
enum RenderBufferUsage : uint32_t
{
    RenderBufferUsageNone           = 0x00,
    RenderBufferUsageTransferSrc    = 0x01,
    RenderBufferUsageTransferDst    = 0x02,
    RenderBufferUsageUniformBuffer  = 0x04,
    RenderBufferUsageStorageBuffer  = 0x08,
    RenderBufferUsageIndexBuffer    = 0x10,
    RenderBufferUsageVertexBuffer   = 0x20,
    RenderBufferUsageIndirectBuffer = 0x40,
};
typedef uint32_t RenderBufferUsageFlags;

/// @brief Render texture types.
enum RenderTextureType : uint32_t
{
    RenderTextureType1D     = 0,
    RenderTextureType2D     = 1,
    RenderTextureType3D     = 2,
};

/// @brief Render texture tiling modes, specifies data layout in memory.
enum RenderTextureTilingMode
{
    RenderTextureTilingLinear   = 0,    /// @brief Lay out data linearly in memory.
    RenderTextureTilingOptimal  = 1,    /// @brief Let the driver decide how to handle the data layout.
};

/// @brief Render texture usage flag bits.
enum RenderTextureUsage : uint32_t
{
    RenderTextureUsageNone                  = 0x00,
    RenderTextureUsageTransferSrc           = 0x01,
    RenderTextureUsageTransferDst           = 0x02,
    RenderTextureUsageSampled               = 0x04,
    RenderTextureUsageStorage               = 0x08,
    RenderTextureUsageRenderTarget          = 0x10,
    RenderTextureUsageDepthStencilTarget    = 0x20,
};
typedef uint32_t RenderTextureUsageFlags;

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
    /// @param imgui_context ImGui context to use for the render backend.
    /// @return A new render backend, or nullptr if no backend is active.
    static RenderBackend* create(PlatformSurface* platform_surface, ImGuiContext* imgui_context);

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
    /// @param size Buffer size in bytes.
    /// @param buffer_usage Buffer usage flags.
    /// @param can_map Indicates if this buffer can be mapped to host memory.
    /// @return a new render buffer object, or nullptr on failure.
    [[nodiscard]]
    virtual RenderBuffer* create_buffer(
        size_t size,
        RenderBufferUsageFlags buffer_usage,
        bool can_map
    ) = 0;

    /// @brief Create a render texture.
    /// @param texture_type Texture type to create.
    /// @param format Format to use for the texture.
    /// @param width Texture width in pixels.
    /// @param height Texture height in pixels.
    /// @param depth_or_layers Texture depth, or layers if a non-3D texture type.
    /// @param mip_levels Number of mip levels to use for this texture.
    /// @param sample_count Number of samples to use for this image, must be a multiple of 2.
    /// @param texture_usage Texture usage flags.
    /// @param tiling_mode Texture tiling mode.
    /// @return A new render texture object, or nullptr on failure.
    [[nodiscard]]
    virtual RenderTexture* create_texture(
        RenderTextureType texture_type,
        RenderFormat format,
        uint32_t width,
        uint32_t height,
        uint32_t depth_or_layers,
        uint32_t mip_levels,
        uint32_t sample_count,
        RenderTextureUsageFlags texture_usage,
        RenderTextureTilingMode tiling_mode
    ) = 0;

    /// @brief Get the current frame index.
    /// @return The currently active frame index.
    [[nodiscard]]
    virtual uint64_t get_current_frame_index() const = 0;
};

#endif //BONSAI_RENDERER_RENDER_BACKEND_HPP