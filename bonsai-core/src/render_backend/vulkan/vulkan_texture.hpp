#pragma once
#ifndef BONSAI_RENDERER_VULKAN_TEXTURE_HPP
#define BONSAI_RENDERER_VULKAN_TEXTURE_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include "bonsai/render_backend/render_backend.hpp"

class VulkanTexture : public RenderTexture
{
public:
    VulkanTexture(VmaAllocator allocator, VkImage image, VmaAllocation allocation);
    ~VulkanTexture() override;

    VulkanTexture(VulkanTexture const&) = delete;
    VulkanTexture& operator=(VulkanTexture const&) = delete;

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
};

#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP