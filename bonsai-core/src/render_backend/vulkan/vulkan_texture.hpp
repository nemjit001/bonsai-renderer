#pragma once
#ifndef BONSAI_RENDERER_VULKAN_TEXTURE_HPP
#define BONSAI_RENDERER_VULKAN_TEXTURE_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include "bonsai/render_backend/render_backend.hpp"

class VulkanTexture : public RenderTexture
{
public:
    VulkanTexture(VkImage image, VkImageView image_view);
    VulkanTexture(VkDevice device, VmaAllocator allocator, VkImage image, VkImageView image_view, VmaAllocation allocation);
    ~VulkanTexture() override;

    VulkanTexture(VulkanTexture const&) = delete;
    VulkanTexture& operator=(VulkanTexture const&) = delete;

    /// @brief Set the next tracked vulkan image layout.
    /// @param next_layout Next layout.
    /// @return The previous image layout.
    [[nodiscard]]
    VkImageLayout set_next_layout(VkImageLayout next_layout);

    VkImage get_image() const { return m_image; }

    VkImageView get_image_view() const { return m_image_view; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_image_view = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP