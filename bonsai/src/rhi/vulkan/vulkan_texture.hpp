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
    VulkanTexture(VmaAllocator allocator, VkImage image, VmaAllocation allocation);
    ~VulkanTexture() override;

    VulkanTexture(VulkanTexture const&) = delete;
    VulkanTexture& operator=(VulkanTexture const&) = delete;

protected:
    void* get_raw_object() const override { return m_image; }

private:
    VmaAllocator    m_allocator     = VK_NULL_HANDLE;
    VkImage         m_image         = VK_NULL_HANDLE;
    VmaAllocation   m_allocation    = VK_NULL_HANDLE;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_TEXTURE_HPP