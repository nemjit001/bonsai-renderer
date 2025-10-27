#include "vulkan_buffer.hpp"
#if BONSAI_USE_VULKAN

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, BufferDesc const& desc)
    :
    IBuffer(),
    m_allocator(allocator),
    m_buffer(buffer),
    m_allocation(allocation),
    m_desc(desc)
{
    //
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}

VkBufferUsageFlags VulkanBuffer::get_vulkan_usage_flags(BufferUsageFlags usage_flags)
{
    VkBufferUsageFlags buffer_usage = 0;
    if (usage_flags & BufferUsageTransferSrc)
    {
        buffer_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (usage_flags & BufferUsageTransferDst)
    {
        buffer_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (usage_flags & BufferUsageUniformTexelBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (usage_flags & BufferUsageStorageTexelBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (usage_flags & BufferUsageUniformBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (usage_flags & BufferUsageStorageBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (usage_flags & BufferUsageIndexBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (usage_flags & BufferUsageVertexBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (usage_flags & BufferUsageIndirectBuffer)
    {
        buffer_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }

    return buffer_usage;
}

#endif //BONSAI_USE_VULKAN