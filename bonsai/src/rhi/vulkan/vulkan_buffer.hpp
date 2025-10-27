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
    VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, BufferDesc const& desc);
    ~VulkanBuffer() override;

    VulkanBuffer(VulkanBuffer const&) = delete;
    VulkanBuffer& operator=(VulkanBuffer const&) = delete;

    /// @brief Convert BufferUsageFlags to Vulkan buffer usage flags.
    /// @param usage_flags
    /// @return
    static VkBufferUsageFlags get_vulkan_usage_flags(BufferUsageFlags usage_flags);

    size_t size() const override { return m_desc.size; }

    BufferDesc get_desc() const override { return m_desc; }

protected:
    void* get_raw_object() const override { return m_buffer; }

private:
    VmaAllocator    m_allocator     = VK_NULL_HANDLE;
    VkBuffer        m_buffer        = VK_NULL_HANDLE;
    VmaAllocation   m_allocation    = VK_NULL_HANDLE;
    BufferDesc      m_desc          = {};
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_BUFFER_HPP