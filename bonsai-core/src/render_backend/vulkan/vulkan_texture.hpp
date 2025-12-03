#pragma once
#ifndef BONSAI_RENDERER_VULKAN_TEXTURE_HPP
#define BONSAI_RENDERER_VULKAN_TEXTURE_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include "bonsai/render_backend/render_backend.hpp"

/// @brief Texture description, stores metadata used to create a texture.
struct VulkanTextureDesc
{
    VkFormat format;
    RenderExtent3D extent;
};

class VulkanTexture : public RenderTexture
{
public:
    VulkanTexture(VkImage image, VkImageView image_view, VulkanTextureDesc desc);
    VulkanTexture(VkDevice device, VmaAllocator allocator, VkImage image, VkImageView image_view, VmaAllocation allocation, VulkanTextureDesc desc);
    ~VulkanTexture() override;

    VulkanTexture(VulkanTexture const&) = delete;
    VulkanTexture& operator=(VulkanTexture const&) = delete;

    RenderExtent3D extent() const override { return m_desc.extent; }

    /// @brief Set the next tracked vulkan image layout.
    /// @param next_layout Next layout.
    /// @return The previous image layout.
    [[nodiscard]]
    VkImageLayout set_next_layout(VkImageLayout next_layout);

    /// @brief Get the current tracked vulkan image layout.
    /// @return The current image layout.
    [[nodiscard]]
    VkImageLayout get_current_layout() const { return m_layout; }

    [[nodiscard]]
    VkImage get_image() const { return m_image; }

    [[nodiscard]]
    VkImageView get_image_view() const { return m_image_view; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_image_view = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VulkanTextureDesc m_desc = {};
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP