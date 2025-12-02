#pragma once
#ifndef BONSAI_RENDERER_VULKAN_BUFFER_HPP
#define BONSAI_RENDERER_VULKAN_BUFFER_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include "bonsai/render_backend/render_backend.hpp"

class VulkanBuffer : public RenderBuffer
{
public:
    VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);
    ~VulkanBuffer() override;

    VulkanBuffer(VulkanBuffer const&) = delete;
    VulkanBuffer& operator=(VulkanBuffer const&) = delete;

    bool map(void** data, size_t size, size_t offset) override;

    void unmap() override;

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
};

#endif //BONSAI_RENDERER_VULKAN_BUFFER_HPP