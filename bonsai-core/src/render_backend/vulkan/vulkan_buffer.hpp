#pragma once
#ifndef BONSAI_RENDERER_VULKAN_BUFFER_HPP
#define BONSAI_RENDERER_VULKAN_BUFFER_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include "bonsai/render_backend/render_backend.hpp"

/// @brief Buffer description, stores metadata used to create a buffer.
struct VulkanBufferDesc
{
    size_t size;
};

class VulkanBuffer : public RenderBuffer
{
public:
    VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, VulkanBufferDesc desc);
    ~VulkanBuffer() override;

    VulkanBuffer(VulkanBuffer const&) = delete;
    VulkanBuffer& operator=(VulkanBuffer const&) = delete;

    size_t size() const override { return m_desc.size; }

    bool map(void** data, size_t size, size_t offset) override;

    void unmap() override;

    /// @brief Get the underlying Vulkan buffer.
    /// @return The underlying Vulkan buffer handle.
    [[nodiscard]]
    VkBuffer get_buffer() const { return m_buffer; }

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VulkanBufferDesc m_desc = {};
};

#endif //BONSAI_RENDERER_VULKAN_BUFFER_HPP