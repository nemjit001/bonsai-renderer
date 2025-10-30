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
    Undefined,

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
    ObjectType get_object() const { return static_cast<ObjectType>(get_raw_object()); }

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
    /// @brief Get the buffer size in bytes.
    virtual size_t size() const = 0;

    /// @brief Get the buffer descriptor that was used to create this buffer.
    /// @return
    virtual BufferDesc get_desc() const = 0;
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

/// @brief The texture tiling modes, only linear tiling textures can be written to directly from the host.
enum class TextureTiling
{
    Optimal,
    Linear,
};

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
    TextureTiling tiling;
    TextureUsageFlags usage;
};

/// @brief Backend texture interface, represents texture resources.
class ITexture : public IResource
{
public:
    /// @brief Get the texture descriptor that was used to create this texture.
    /// @return
    virtual TextureDesc get_desc() const = 0;
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
    /// @brief Begin recording commands on this command buffer.
    /// @return True on successful recording start, false otherwise.
    virtual bool begin() = 0;

    /// @brief Close this command buffer, finalizing command recording.
    /// @return True on successful close, false otherwise.
    virtual bool close() = 0;
};
using CommandBufferHandle = std::shared_ptr<ICommandBuffer>;

/// @brief Backend command allocator type, used to allocate command buffers for a queue.
class ICommandAllocator : public IResource
{
public:
    /// @brief Reset the command allocator, resetting all allocated command buffers in the process.
    /// @return True on success, false otherwise.
    virtual bool reset() = 0;

    /// @brief Create a new command buffer from this command allocator.
    /// @return A new command buffer handle.
    virtual CommandBufferHandle create_command_buffer() = 0;
};
using CommandAllocatorHandle = std::shared_ptr<ICommandAllocator>;

/// @brief Swap chain present modes for present synchronization.
enum class SwapPresentMode
{
    FiFo,
    Mailbox,
    Immediate,
};

/// @brief Swap chain description for swap chain management for non-headless render devices.
struct SwapChainDesc
{
    Surface* surface;
    uint32_t image_count;
    Format format;
    uint32_t width;
    uint32_t height;
    TextureUsageFlags usage;
    SwapPresentMode present_mode;
};

/// @brief Swap chain type, used to manage swap images for a render device on a given surface.
class ISwapChain : public IResource
{
public:
    /// @brief Resize the swap buffers managed by this swap chain, all references to the swap buffers must be released.
    /// @param width New width in pixels.
    /// @param height New height in pixels
    /// @param present_mode Updated present mode for the swap chain.
    /// @return True on successful resize, false otherwise.
    virtual bool resize_swap_buffers(uint32_t width, uint32_t height, SwapPresentMode present_mode) = 0;

    /// @brief Acquire the next swap chain image.
    /// @return True on success, false otherwise.
    virtual bool acquire_next_image() = 0;

    /// @brief Present the next swap chain image.
    /// @return True on success, false otherwise.
    virtual bool present() = 0;

    /// @brief Return the currently acquired image index.
    /// @return
    virtual uint32_t current_image_idx() = 0;

    /// @brief Return the number of swap chain images.
    /// @return
    virtual uint32_t swap_image_count() = 0;

    /// @brief Get a swap image by index.
    /// @param idx Swap image index to retrieve.
    /// @return A TextureHandle for the swap image, or a null handle if the image does not exist (e.g. index was out of range).
    virtual TextureHandle get_swap_image(uint32_t idx) = 0;

    /// @brief Get the swap chain descriptor that represents this swap chain in the current state.
    /// @return
    virtual SwapChainDesc get_desc() const = 0;
};
using SwapChainHandle = std::shared_ptr<ISwapChain>;

/// @brief Render device description for device creation.
struct RenderDeviceDesc
{
    Surface* compatible_surface; /// @brief Optional surface pointer, if set present support for this surface is guaranteed on the device.
    uint32_t frames_in_flight; /// @brief Number of frames to allow simultaneous command recording for.
};

/// @brief Backend render device interface, used for render resource allocation.
class IRenderDevice : public IResource
{
public:
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

    /// @brief Create a command allocator for a command queue.
    /// @param queue Command queue type to use for command allocator.
    /// @return A new command allocator handle.
    virtual CommandAllocatorHandle create_command_allocator(CommandQueueType queue) = 0;

    /// @brief Create a swap chain on this device, the device MUST NOT be running as headless.
    /// @param desc Swap chain descriptor.
    /// @return A new swap chain handle or nullptr if the device was headless.
    virtual SwapChainHandle create_swap_chain(SwapChainDesc const& desc) = 0;

    /// @brief Submit work to a GPU queue on the render device.
    /// @param queue Queue type to submit work to.
    /// @param count Number of command buffers to submit.
    /// @param command_buffers Recorded command buffers to submit.
    virtual void submit(CommandQueueType queue, size_t count, CommandBufferHandle* command_buffers) = 0;

    /// @brief Wait for the device to be idle.
    virtual void wait_idle() = 0;
};
using RenderDeviceHandle = std::shared_ptr<IRenderDevice>;

/// @brief RHI instance, handles graphics API initialization steps needed before device creation.
class IRHIInstance : public IResource
{
public:
    /// @brief Create a render device on the RHI.
    /// @param desc Render device descriptor.
    /// @return A new render device handle.
    virtual RenderDeviceHandle create_render_device(RenderDeviceDesc const& desc) = 0;
};
using RHIInstanceHandle = std::shared_ptr<IRHIInstance>;

namespace rhi
{
    /// @brief Create a new RHI instance.
    RHIInstanceHandle create_instance();
} //namespace rhi

#endif //BONSAI_RENDERER_RHI_HPP