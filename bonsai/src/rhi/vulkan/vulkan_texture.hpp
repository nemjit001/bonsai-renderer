#pragma once
#ifndef BONSAI_RENDERER_VULKAN_TEXTURE_HPP
#define BONSAI_RENDERER_VULKAN_TEXTURE_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include "rhi/rhi.hpp"

class VulkanTextureView : public ITextureView
{
public:
    VulkanTextureView(VkDevice device, VkImageView view);
    ~VulkanTextureView() override;

    VulkanTextureView(VulkanTextureView const&) = delete;
    VulkanTextureView& operator=(VulkanTextureView const&) = delete;

    /// @brief Get the Vulkan image view type for a texture view type.
    /// @param view_type
    /// @return
    static VkImageViewType get_vulkan_view_type(TextureViewType view_type);

    /// @brief Get the Vulkan image aspect flags for a given format.
    /// @param format
    /// @return
    static VkImageAspectFlags get_vulkan_aspect_flags(Format format);

protected:
    void* get_raw_object() const override { return m_view; }

private:
    VkDevice    m_device    = VK_NULL_HANDLE;
    VkImageView m_view      = VK_NULL_HANDLE;
};

class VulkanTexture : public ITexture
{
public:
    /// Create an imported VulkanTexture without access to the backing allocator, useful for e.g. swap chain image handles.
    /// @param device Device to use for view creation.
    /// @param image Image to use for texture creation.
    /// @param desc Descriptor that matches the image creation parameters.
    VulkanTexture(VkDevice device, VkImage image, TextureDesc const& desc);

    /// @brief Create an allocated VulkanTexture.
    /// @param device Device used to create allocator.
    /// @param allocator Allocator used for allocation.
    /// @param image Managed image.
    /// @param allocation Associated image memory.
    /// @param desc Texture descriptor used to create image.
    VulkanTexture(VkDevice device, VmaAllocator allocator, VkImage image, VmaAllocation allocation, TextureDesc const& desc);
    ~VulkanTexture() override;

    VulkanTexture(VulkanTexture const&) = delete;
    VulkanTexture& operator=(VulkanTexture const&) = delete;

    /// @brief Get the Vulkan image type for a texture type.
    /// @param texture_type
    /// @return
    static VkImageType get_vulkan_image_type(TextureType texture_type);

    /// @brief Get the Vulkan image format for a format type.
    /// @param format
    /// @return
    static VkFormat get_vulkan_format(Format format);

    /// @brief Get the Vulkan image usage flags for texture usage flags.
    /// @param usage_flags
    /// @return
    static VkImageUsageFlags get_vulkan_usage_flags(TextureUsageFlags usage_flags);

    /// @brief Get the Vulkan sample count flags for a given sample count.
    /// @param sample_count MUST be a multiple of 2 in range [1, 64]
    /// @return
    static VkSampleCountFlagBits get_vulkan_sample_count(size_t sample_count);

    /// @brief Get the Vulkan tiling value for a given texture tiling.
    /// @param tiling
    /// @return
    static VkImageTiling get_vulkan_image_tiling(TextureTiling tiling);

    /// @brief Get the VUlkan image layout for a texture layout.
    /// @param layout
    /// @return
    static VkImageLayout get_vulkan_image_layout(TextureLayout layout);

    TextureViewHandle create_view(TextureViewDesc const* view_desc) override;

    TextureType type() const override { return m_desc.type; }

    Format fomat() const override { return m_desc.format; }

    uint32_t width() const override { return m_desc.width; }

    uint32_t height() const override { return m_desc.height; }

    uint32_t depth() const override { return m_desc.type != TextureType::Type3D ? 1u : static_cast<uint32_t>(m_desc.depth_or_layers); }

    uint32_t layers() const override { return m_desc.type == TextureType::Type3D ? 1u : static_cast<uint32_t>(m_desc.depth_or_layers); }

    uint32_t mip_levels() const override { return m_desc.mip_levels; }

    TextureDesc get_desc() const override { return m_desc; }

protected:
    void* get_raw_object() const override { return m_image; }

private:
    bool            m_imported      = false;
    VkDevice        m_device        = VK_NULL_HANDLE;
    VmaAllocator    m_allocator     = VK_NULL_HANDLE;
    VkImage         m_image         = VK_NULL_HANDLE;
    VmaAllocation   m_allocation    = VK_NULL_HANDLE;
    TextureDesc     m_desc          = {};
};

#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP