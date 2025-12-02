#include "vulkan_buffer.hpp"

#include "bonsai/core/assert.hpp"
#include "render_backend/vulkan/vk_check.hpp"

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, VulkanBufferDesc desc)
    :
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

bool VulkanBuffer::map(void** data, [[maybe_unused]] size_t size, size_t offset)
{
    BONSAI_ASSERT(data != nullptr && "Pointer to data block was NULL!");
    if (VK_FAILED(vmaMapMemory(m_allocator, m_allocation, data)))
    {
        return false;
    }

    *data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(data) + offset);
    return true;
}

void VulkanBuffer::unmap()
{
    vmaUnmapMemory(m_allocator, m_allocation);
}
