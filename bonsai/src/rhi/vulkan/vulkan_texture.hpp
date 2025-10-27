#pragma once
#ifndef BONSAI_RENDERER_VULKAN_TEXTURE_HPP
#define BONSAI_RENDERER_VULKAN_TEXTURE_HPP
#if BONSAI_USE_VULKAN

#include <volk.h>
#include <vk_mem_alloc.h>
#include "rhi/rhi.hpp"

class VulkanTexture : public ITexture
{
public:
    /// Create an imported VulkanTexture without access to the backing allocator, useful for e.g. swap chain image handles.
    /// @param image Image to use for texture creation.
    /// @param desc Descriptor that matches the image creation parameters.
    VulkanTexture(VkImage image, TextureDesc const& desc);

    /// @brief Create an allocated VulkanTexture.
    /// @param allocator Allocator used for allocation.
    /// @param image Managed image.
    /// @param allocation Associated image memory.
    /// @param desc Texture descriptor used to create image.
    VulkanTexture(VmaAllocator allocator, VkImage image, VmaAllocation allocation, TextureDesc const& desc);
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

protected:
    void* get_raw_object() const override { return m_image; }

private:
    bool            m_imported      = false;
    VmaAllocator    m_allocator     = VK_NULL_HANDLE;
    VkImage         m_image         = VK_NULL_HANDLE;
    VmaAllocation   m_allocation    = VK_NULL_HANDLE;
    TextureDesc     m_desc          = {};
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP