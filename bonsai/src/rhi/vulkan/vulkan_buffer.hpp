#pragma once
#ifndef BONSAI_RENDERER_VULKAN_BUFFER_HPP
#define BONSAI_RENDERER_VULKAN_BUFFER_HPP
#if BONSAI_USE_VULKAN

#include <volk.h>
#include <vk_mem_alloc.h>
#include "rhi/rhi.hpp"

class VulkanBuffer : public IBuffer
{
public:
    VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, size_t size);
    ~VulkanBuffer();

    VulkanBuffer(VulkanBuffer const&) = delete;
    VulkanBuffer& operator=(VulkanBuffer const&) = delete;

    size_t size() const override { return m_buffer_size; }

protected:
    void* get_raw_object() const override { return m_buffer; }

private:
    VmaAllocator    m_allocator     = VK_NULL_HANDLE;
    VkBuffer        m_buffer        = VK_NULL_HANDLE;
    VmaAllocation   m_allocation    = VK_NULL_HANDLE;
    size_t          m_buffer_size   = 0;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_BUFFER_HPP