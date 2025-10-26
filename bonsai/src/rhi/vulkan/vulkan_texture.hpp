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
    VulkanTexture(VmaAllocator allocator, VkImage image, VmaAllocation allocation, TextureDesc const& desc);
    ~VulkanTexture() override;

    VulkanTexture(VulkanTexture const&) = delete;
    VulkanTexture& operator=(VulkanTexture const&) = delete;

    /// @brief Get the Vulkan image type for a texture type.
    /// @param texture_type
    /// @return
    static VkImageType get_vulkan_image_type(TextureType texture_type);

    /// @brief Get the Vulkan image usage flags for texture usage flags.
    /// @param usage_flags
    /// @return
    static VkImageUsageFlags get_vulkan_usage_flags(TextureUsageFlags usage_flags);

    /// @brief Get the Vulkan sample count flags for a given sample count.
    /// @param sample_count MUST be a multiple of 2 in range [1, 64]
    /// @return
    static VkSampleCountFlagBits get_vulkan_sample_count(size_t sample_count);

protected:
    void* get_raw_object() const override { return m_image; }

private:
    VmaAllocator    m_allocator     = VK_NULL_HANDLE;
    VkImage         m_image         = VK_NULL_HANDLE;
    VmaAllocation   m_allocation    = VK_NULL_HANDLE;
    TextureDesc     m_desc          = {};
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP