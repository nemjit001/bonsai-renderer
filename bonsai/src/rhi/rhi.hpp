#pragma once
#ifndef BONSAI_RENDERER_RHI_HPP
#define BONSAI_RENDERER_RHI_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include "platform/platform.hpp"

/// @brief Available data format values, used for textures or data layout specification.
enum class Format
{
    Undefined = 0,

    R8_UINT,
    R8_SINT,
    R8_UNORM,
    R8_SNORM,
    RG8_UINT,
    RG8_SINT,
    RG8_UNORM,
    RG8_SNORM,
    RGBA8_UINT,
    RGBA8_SINT,
    RGBA8_UNORM,
    RGBA8_SNORM,
    RGBA8_UNORM_SRGB,
    BGRA8_UNORM,
    BGRA8_UNORM_SRGB,

    R16_UINT,
    R16_SINT,
    R16_UNORM,
    R16_SNORM,
    R16_FLOAT,
    RG16_UINT,
    RG16_SINT,
    RG16_UNORM,
    RG16_SNORM,
    RG16_FLOAT,
    RGBA16_UINT,
    RGBA16_SINT,
    RGBA16_UNORM,
    RGBA16_SNORM,
    RGBA16_FLOAT,

    R32_UINT,
    R32_SINT,
    R32_FLOAT,
    RG32_UINT,
    RG32_SINT,
    RG32_FLOAT,
    RGB32_UINT,
    RGB32_SINT,
    RGB32_FLOAT,
    RGBA32_UINT,
    RGBA32_SINT,
    RGBA32_FLOAT,

    Depth16,
    Depth24Stencil8,
    Depth32,
    Depth32Stencil8,
};

/// @brief RHI resource interface, provides access to internal render types.
class IResource
{
public:
    virtual ~IResource() = default;

    template<typename ObjectType>
    ObjectType* get_object() const { return static_cast<ObjectType*>(get_raw_object()); }

protected:
    /// @brief Get the underlying graphics API object for this resource.
    /// @return An opaque pointer to the underlying resource object.
    virtual void* get_raw_object() const { return nullptr; }
};

/// @brief Buffer usage values.
enum BufferUsage
{
    BufferUsageTransferSrc          = 0x0001,
    BufferUsageTransferDst          = 0x0002,
    BufferUsageUniformTexelBuffer   = 0x0004,
    BufferUsageStorageTexelBuffer   = 0x0008,
    BufferUsageUniformBuffer        = 0x0010,
    BufferUsageStorageBuffer        = 0x0020,
    BufferUsageIndexBuffer          = 0x0040,
    BufferUsageVertexBuffer         = 0x0080,
    BufferUsageIndirectBuffer       = 0x0100,
};
typedef uint32_t BufferUsageFlags;

/// @brief Buffer description for resource creation.
struct BufferDesc
{
    size_t size;
    BufferUsageFlags usage;
};

/// @brief Backend buffer interface, represents buffer resources.
class IBuffer : public IResource
{
public:
    virtual size_t size() const = 0;
};
using BufferHandle = std::shared_ptr<IBuffer>;

/// @brief Texture type values.
enum class TextureType
{
    Type1D,
    Type2D,
    Type3D,
};

/// @brief Texture usage values.
enum TextureUsage
{
    TextureUsageTransferSrc             = 0x01,
    TextureUsageTransferDst             = 0x02,
    TextureUsageSampled                 = 0x04,
    TextureUsageStorage                 = 0x08,
    TextureUsageColorAttachment         = 0x10,
    TextureUsageDepthStencilAttachment  = 0x20,
};
typedef uint32_t TextureUsageFlags;

/// @brief Texture description for resource creation.
struct TextureDesc
{
    TextureType type;
    Format format;
    size_t width;
    size_t height;
    size_t depth_or_layers;
    size_t mip_levels;
    size_t sample_count;
    TextureUsageFlags usage;
};

/// @brief Backend texture interface, represents texture resources.
class ITexture : public IResource
{
public:
    //
};
using TextureHandle = std::shared_ptr<ITexture>;

/// @brief Available command queue types.
enum class CommandQueueType : uint8_t
{
    Direct      = 0x1,
    Transfer    = 0x2,
    Compute     = 0x4,
    All = Direct | Transfer | Compute,
};

/// @brief Backend command buffer type, used to record render commands.
class ICommandBuffer : public IResource
{
public:
    //
};
using CommandBufferHandle = std::shared_ptr<ICommandBuffer>;

/// @brief Render device description for device creation.
struct RenderDeviceDesc
{
    Surface* compatible_surface; /// @brief Optional surface pointer, if set present support for this surface is guaranteed on the device.
};

/// @brief Backend render device interface, used for render resource allocation.
class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;

    /// @brief Check if this render device was created as a headless device (i.e. no compatible surface was specified).
    /// @return A boolean indicating headless state.
    virtual bool is_headless() const = 0;

    /// @brief Create a buffer resource.
    /// @param desc Buffer resource descriptor.
    /// @return A new buffer resource handle.
    virtual BufferHandle create_buffer(BufferDesc& desc) = 0;

    /// @brief Create a texture resource.
    /// @param desc Texture resource descriptor.
    /// @return A new texture resource handle.
    virtual TextureHandle create_texture(TextureDesc& desc) = 0;
};
using RenderDeviceHandle = std::shared_ptr<IRenderDevice>;

/// @brief RHI instance, handles graphics API initialization steps needed before device creation.
class IRHIInstance
{
public:
    virtual ~IRHIInstance() = default;

    /// @brief Create a render device on the RHI.
    /// @param desc Render device descriptor.
    /// @return A new render device handle.
    virtual RenderDeviceHandle create_render_device(RenderDeviceDesc const& desc) = 0;
};
using RHIInstanceHandle = std::shared_ptr<IRHIInstance>;

/// @brief Create a new RHI instance.
RHIInstanceHandle create_rhi_instance();

#endif //BONSAI_RENDERER_RHI_HPP