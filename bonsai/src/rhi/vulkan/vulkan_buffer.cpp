#include "vulkan_buffer.hpp"

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, size_t size)
    :
    IBuffer(),
    m_allocator(allocator),
    m_buffer(buffer),
    m_allocation(allocation),
    m_buffer_size(size)
{
    //
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}
